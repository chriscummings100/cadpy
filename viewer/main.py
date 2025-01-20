# SPDX-License-Identifier: Apache-2.0

import sgl
from pathlib import Path
import app
from graphicsmesh import GraphicsMesh
import numpy as np
import cadpy as cp

app.init()

def mulvec(m, v):
    f4 = sgl.math.mul(m, sgl.float4(v, 0))
    return sgl.float3(f4.x, f4.y, f4.z)

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

        self.ui = sgl.ui.Context(app.device)

        self.output_texture = None
        self.output_depth = None
        self.framebuffer = None

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

        self.cube = cp.BSP.cube(cp.float3(1, 1, 1))

        plane = cp.Plane()
        plane.normal = cp.float3(1, 0, 0)
        plane.d = 0.5
        self.cube.split(plane)


        self.edges = self.cube.to_edge_mesh()
        self.triangles = self.cube.to_tri_mesh()

        self.edge_gfx = GraphicsMesh()
        self.edge_gfx.vertices = self.edges.positions
        self.edge_gfx.normals = self.edges.normals
        self.edge_gfx.colors = self.edges.colors
        self.edge_gfx.indices = self.edges.indices
        self.triangles_gfx = GraphicsMesh()
        self.triangles_gfx.vertices = self.triangles.positions
        self.triangles_gfx.normals = self.triangles.normals
        self.triangles_gfx.colors = self.triangles.colors
        self.triangles_gfx.indices = self.triangles.indices

        self.camera_pos = sgl.float3(0, 0, 0)
        self.camera_dir = sgl.math.normalize(sgl.float3(0, 0, 1))
        self.camera_up = sgl.float3(0, 1, 0)

        self.keys = set()

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
            self.keys.add(event.key)
        elif event.type == sgl.KeyboardEventType.key_release:
            try:
                self.keys.remove(event.key)
            except KeyError:
                pass

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

            if sgl.KeyCode.a in self.keys:
                self.camera_pos -= 0.1 * sgl.math.cross(self.camera_dir, self.camera_up)
            if sgl.KeyCode.d in self.keys:
                self.camera_pos += 0.1 * sgl.math.cross(self.camera_dir, self.camera_up)
            if sgl.KeyCode.w in self.keys:
                self.camera_pos += 0.1 * self.camera_dir
            if sgl.KeyCode.s in self.keys:
                self.camera_pos -= 0.1 * self.camera_dir
            if sgl.KeyCode.q in self.keys:
                self.camera_pos -= 0.1 * self.camera_up
            if sgl.KeyCode.e in self.keys:
                self.camera_pos += 0.1 * self.camera_up
            if sgl.KeyCode.left in self.keys:
                rot = sgl.math.matrix_from_rotation(0.01, self.camera_up)
                self.camera_dir = mulvec(rot, self.camera_dir)
            if sgl.KeyCode.right in self.keys:
                rot = sgl.math.matrix_from_rotation(-0.01,self.camera_up)
                self.camera_dir = mulvec(rot, self.camera_dir)
            if sgl.KeyCode.up in self.keys:
                rot = sgl.math.matrix_from_rotation(-0.01, sgl.math.cross(self.camera_dir, self.camera_up))
                self.camera_dir = mulvec(rot, self.camera_dir)
                #self.camera_up = mulvec(rot, self.camera_up)
            if sgl.KeyCode.down in self.keys:
                rot = sgl.math.matrix_from_rotation(0.01, sgl.math.cross(self.camera_dir, self.camera_up))
                self.camera_dir = mulvec(rot, self.camera_dir)
                #self.camera_up = mulvec(rot, self.camera_up)

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
                    | sgl.ResourceUsage.unordered_access
                    | sgl.ResourceUsage.render_target,
                    debug_name="output_texture",
                )
                self.output_depth = app.device.create_texture(
                    format=sgl.Format.d32_float_s8_uint,
                    width=image.width,
                    height=image.height,
                    mip_count=1,
                    usage=sgl.ResourceUsage.depth_stencil,
                    debug_name="output_depth",
                )
                self.framebuffer = app.device.create_framebuffer(render_targets=[self.output_texture.get_rtv()], depth_stencil=self.output_depth.get_dsv())
                self.edge_gfx.build_pipeline(self.program, self.framebuffer)
                self.triangles_gfx.build_pipeline(self.program, self.framebuffer)

            world_from_camera = sgl.math.matrix_from_look_at(self.camera_pos,self.camera_pos+self.camera_dir,self.camera_up,sgl.math.Handedness.right_handed)
            view_from_world = world_from_camera #sgl.math.inverse(world_from_camera)
            proj_from_view = sgl.math.perspective(1.5, image.width/image.height, 0.1, 1000)

            self.edge_gfx.world_from_local = sgl.math.matrix_from_translation(sgl.float3(0,0,5)) #sgl.math.mul(sgl.math.matrix_from_translation(sgl.float3(0,0,5)), sgl.math.matrix_from_rotation_y(time*0.1))
            self.triangles_gfx.world_from_local = self.edge_gfx.world_from_local

            command_buffer = app.device.create_command_buffer()
            command_buffer.clear_resource_view(self.output_texture.get_rtv(), [0.0, 0.0, 0.1, 1.0])
            command_buffer.clear_resource_view(self.output_depth.get_dsv(), 1, 0, True, True)

            with command_buffer.encode_render_commands(self.framebuffer) as encoder:

                shader_object = encoder.bind_pipeline(self.triangles_gfx.pipeline)
                cursor = sgl.ShaderCursor(shader_object)
                cursor.g_world_from_local = self.triangles_gfx.world_from_local
                cursor.g_proj_from_local = sgl.math.mul(sgl.math.mul(proj_from_view,view_from_world),self.triangles_gfx.world_from_local)
                cursor.g_lit = True
                encoder.set_vertex_buffer(0, self.triangles_gfx.buffers.vertex_buffer)
                encoder.set_index_buffer(self.triangles_gfx.buffers.index_buffer, sgl.Format.r32_uint)
                encoder.set_primitive_topology(sgl.PrimitiveTopology.triangle_list)
                encoder.set_viewport_and_scissor_rect(
                    {"width": image.width, "height": image.height}
                )
                encoder.draw_indexed(self.triangles_gfx.indices.size)

                shader_object = encoder.bind_pipeline(self.edge_gfx.pipeline)
                cursor = sgl.ShaderCursor(shader_object)
                cursor.g_world_from_local = self.edge_gfx.world_from_local
                cursor.g_proj_from_local = sgl.math.mul(sgl.math.mul(proj_from_view,view_from_world),self.edge_gfx.world_from_local)
                cursor.g_lit = False
                encoder.set_vertex_buffer(0, self.edge_gfx.buffers.vertex_buffer)
                encoder.set_index_buffer(self.edge_gfx.buffers.index_buffer, sgl.Format.r32_uint)
                encoder.set_primitive_topology(sgl.PrimitiveTopology.line_list)
                encoder.set_viewport_and_scissor_rect(
                    {"width": image.width, "height": image.height}
                )
                encoder.draw_indexed(self.edge_gfx.indices.size)

            command_buffer.submit()

            command_buffer = app.device.create_command_buffer()
            self.ui.new_frame(image.width, image.height)
            self.ui.render(self.framebuffer, command_buffer)

            command_buffer.blit(image, self.output_texture)
            command_buffer.set_texture_state(image, sgl.ResourceState.present)
            command_buffer.submit()
            del image

            self.swapchain.present()
            app.device.run_garbage_collection()
            frame += 1


if __name__ == "__main__":
    x = App()
    x.run()
