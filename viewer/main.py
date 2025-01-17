# SPDX-License-Identifier: Apache-2.0

import sgl
from pathlib import Path
import app
from mesh import Mesh
import numpy as np

app.init()


class App:
    def __init__(self):
        super().__init__()
        self.window = sgl.Window(
            width=1920, height=1280, title="Example", resizable=True
        )
        self.swapchain = app.device.create_swapchain(
            image_count=3,
            width=self.window.width,
            height=self.window.height,
            window=self.window,
            enable_vsync=False,
        )

        self.framebuffers = []
        self.create_framebuffers()

        self.ui = sgl.ui.Context(app.device)

        self.output_texture = None

        self.program = app.device.load_program(
            "render.slang", ["vertex_main", "fragment_main"]
        )

        self.mouse_pos = sgl.float2()
        self.mouse_down = False

        self.playing = True
        self.fps_avg = 0.0

        self.window.on_keyboard_event = self.on_keyboard_event
        self.window.on_mouse_event = self.on_mouse_event
        self.window.on_resize = self.on_resize

        self.mesh = Mesh()
        self.mesh.vertices = np.array([[-1, -1, 0], [-1, 1, 0], [1, 0, 0]], dtype=np.float32)
        self.mesh.normals = np.array([[1, 0, 0], [1, 0, 0], [1, 0, 0]], dtype=np.float32)
        self.mesh.indices = np.array([0, 1, 2], dtype=np.uint32)
        self.mesh.build_buffers(app.device)

        self.pipeline = self.mesh.build_pipeline(self.program, self.framebuffers[0])

        self.setup_ui()

    def setup_ui(self):
        screen = self.ui.screen
        window = sgl.ui.Window(screen, "Settings", size=sgl.float2(500, 300))
        self.fps_text = sgl.ui.Text(window, "FPS: 0")

    def on_keyboard_event(self, event: sgl.KeyboardEvent):
        if self.ui.handle_keyboard_event(event):
            return

        if event.type == sgl.KeyboardEventType.key_press:
            if event.key == sgl.KeyCode.escape:
                self.window.close()
            elif event.key == sgl.KeyCode.f1:
                if self.output_texture:
                    sgl.tev.show_async(self.output_texture)
            elif event.key == sgl.KeyCode.f2:
                if self.output_texture:
                    bitmap = self.output_texture.to_bitmap()
                    bitmap.convert(
                        sgl.Bitmap.PixelFormat.rgb,
                        sgl.Bitmap.ComponentType.uint8,
                        srgb_gamma=True,
                    ).write_async("screenshot.png")

    def on_mouse_event(self, event: sgl.MouseEvent):
        if self.ui.handle_mouse_event(event):
            return

        if event.type == sgl.MouseEventType.move:
            self.mouse_pos = event.pos
        elif event.type == sgl.MouseEventType.button_down:
            if event.button == sgl.MouseButton.left:
                self.mouse_down = True
        elif event.type == sgl.MouseEventType.button_up:
            if event.button == sgl.MouseButton.left:
                self.mouse_down = False

    def on_resize(self, width: int, height: int):
        self.framebuffers.clear()
        app.device.wait()
        self.swapchain.resize(width, height)
        self.create_framebuffers()

    def create_framebuffers(self):
        self.framebuffers = [
            app.device.create_framebuffer(render_targets=[image.get_rtv()])
            for image in self.swapchain.images
        ]

    def run(self):
        frame = 0
        time = 0.0
        timer = sgl.Timer()

        while not self.window.should_close():
            self.window.process_events()
            self.ui.process_events()

            elapsed = timer.elapsed_s()
            timer.reset()

            if self.playing:
                time += elapsed

            self.fps_avg = 0.95 * self.fps_avg + 0.05 * (1.0 / elapsed)
            self.fps_text.text = f"FPS: {self.fps_avg:.2f}"

            image_index = self.swapchain.acquire_next_image()
            if image_index < 0:
                continue

            image = self.swapchain.get_image(image_index)
            if (
                self.output_texture == None
                or self.output_texture.width != image.width
                or self.output_texture.height != image.height
            ):
                self.output_texture = app.device.create_texture(
                    format=sgl.Format.rgba16_float,
                    width=image.width,
                    height=image.height,
                    mip_count=1,
                    usage=sgl.ResourceUsage.shader_resource
                    | sgl.ResourceUsage.unordered_access,
                    debug_name="output_texture",
                )

            camera_pos = sgl.float3(0, 0, -5)
            camera_dir = sgl.float3(0, 0, 1)

            world_from_camera = sgl.math.matrix_from_look_at(camera_pos,camera_pos+camera_dir,sgl.float3(0,1,0))
            view_from_world = sgl.math.inverse(world_from_camera)
            proj_from_view = sgl.math.perspective(1.5, 1, 0.1, 1000)

            self.mesh.world_from_local = sgl.math.matrix_from_rotation_y(time)

            command_buffer = app.device.create_command_buffer()
            command_buffer.clear_texture(image, sgl.float4(1,0,0,0))
            with command_buffer.encode_render_commands(self.framebuffers[image_index]) as encoder:
                shader_object = encoder.bind_pipeline(self.pipeline)
                cursor = sgl.ShaderCursor(shader_object)
                cursor.g_world_from_local = self.mesh.world_from_local
                cursor.g_proj_from_local = sgl.math.mul(sgl.math.mul(proj_from_view,view_from_world),self.mesh.world_from_local)
                encoder.set_vertex_buffer(0, self.mesh.buffers.vertex_buffer)
                encoder.set_index_buffer(self.mesh.buffers.index_buffer, sgl.Format.r32_uint)
                encoder.set_primitive_topology(sgl.PrimitiveTopology.triangle_list)
                encoder.set_viewport_and_scissor_rect(
                    {"width": image.width, "height": image.height}
                )
                encoder.draw_indexed(3)
            command_buffer.submit()

            command_buffer = app.device.create_command_buffer()
            self.ui.new_frame(image.width, image.height)
            self.ui.render(self.framebuffers[image_index], command_buffer)

            command_buffer.set_texture_state(image, sgl.ResourceState.present)
            command_buffer.submit()
            del image

            self.swapchain.present()
            app.device.run_garbage_collection()
            frame += 1


if __name__ == "__main__":
    x = App()
    x.run()
