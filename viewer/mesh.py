import sgl
import numpy as np

class MeshBuffers:
    def __init__(self) -> None:
        self.vertex_buffer: sgl.Buffer = None
        self.index_buffer: sgl.Buffer = None
        self.input_layout: sgl.InputLayout = None
        self.program: sgl.ShaderProgram = None
        self.index_count: int = 0

class Mesh:

    def __init__(self):
        super().__init__()
        self.world_from_local: sgl.float4x4 = sgl.float4x4.identity()
        self.vertices: np.ndarray = None
        self.normals: np.ndarray = None
        self.tangents: np.ndarray = None
        self.texturecoords: np.ndarray = None
        self.colors: np.ndarray = None
        self.indices: np.ndarray = None
        self.buffers: MeshBuffers = None
        self.pipeline: sgl.GraphicsPipeline = None

    def _els_to_format(self, count):
        if count == 1:
            return sgl.Format.r32_float
        if count == 2:
            return sgl.Format.rg32_float
        if count == 3:
            return sgl.Format.rgb32_float
        if count == 4:
            return sgl.Format.rgba32_float
        raise ValueError(f"unsupported element count {count}")
 
    def build_buffers(self, device: sgl.Device):
        buffers = MeshBuffers()

        input_elements: list[sgl.InputElementDesc] = []
        stream_stride = 0

        dim = self.vertices.shape[1]
        input_elements.append({
            "semantic_name": "POSITION",
            "semantic_index": 0,
            "format": self._els_to_format(dim),
            "offset": stream_stride,
        })
        stream = np.copy(self.vertices)
        stream_stride = 4*dim

        if self.normals is not None and self.normals.size > 0:
            dim = self.normals.shape[1]
            input_elements.append({
                "semantic_name": "NORMAL",
                "semantic_index": 0,
                "format": self._els_to_format(dim),
                "offset": stream_stride,
            })
            stream = np.concatenate((stream, self.normals), axis=1)
            stream_stride += 4*dim

        if self.texturecoords is not None:
            for i in range(self.texturecoords.shape[0]):
                texturecoords = self.texturecoords[i]
                dim = texturecoords.shape[1]
                input_elements.append({
                    "semantic_name": "TEXCOORD",
                    "semantic_index": i,
                    "format": self._els_to_format(dim),
                    "offset": stream_stride,
                })
                stream = np.concatenate((stream, texturecoords), axis=1)
                stream_stride += 4*dim

        buffers.vertex_buffer = device.create_buffer(
            usage=sgl.ResourceUsage.shader_resource,
            debug_name="vertex_buffer",
            data=stream,
        )

        buffers.input_layout = device.create_input_layout(
            input_elements=input_elements,
            vertex_streams=[{"stride": stream_stride}],
        )

        buffers.index_buffer = device.create_buffer(
            usage=sgl.ResourceUsage.shader_resource,
            debug_name="index_buffer",
            data=np.copy(self.indices),
        )
        buffers.index_count = len(self.indices)*3

        self.buffers = buffers
        return buffers

    def build_pipeline(self, program: sgl.ShaderProgram, framebuffer: sgl.Framebuffer):
        if self.buffers is None:
            self.build_buffers(program.device)
        pipeline = program.device.create_graphics_pipeline(
            program=program,
            input_layout=self.buffers.input_layout,
            framebuffer_layout=framebuffer.layout,
            rasterizer={
                "cull_mode": sgl.CullMode.none
            },
            depth_stencil={
                "depth_test_enable": False
            }
        )
        self.pipeline = pipeline
        return pipeline
