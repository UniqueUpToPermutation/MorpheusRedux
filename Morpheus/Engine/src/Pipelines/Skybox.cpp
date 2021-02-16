#include <Engine/Pipelines/PipelineFactory.hpp>
#include <Engine/Resources/PipelineResource.hpp>
#include <Engine/Resources/ShaderResource.hpp>
#include <Engine/Resources/ResourceManager.hpp>
#include <Engine/Renderer.hpp>

namespace Morpheus {
	TaskId CreateSkyboxPipeline(DG::IRenderDevice* device,
		ResourceManager* manager,
		IRenderer* renderer,
		PipelineResource* into,
		const ShaderPreprocessorConfig* overrides,
		const AsyncResourceParams* asyncParams) {

		LoadParams<ShaderResource> vsParams(
			"internal/Skybox.vsh",
			DG::SHADER_TYPE_VERTEX,
			"Skybox VS",
			overrides,
			"main"
		);

		LoadParams<ShaderResource> psParams(
			"internal/Skybox.psh",
			DG::SHADER_TYPE_PIXEL,
			"Skybox PS",
			overrides,
			"main"
		);

		ShaderResource* skyboxVSResource = nullptr;
		ShaderResource* skyboxPSResource = nullptr;
		TaskBarrier* postLoadBarrier = into->GetLoadBarrier();
		TaskId loadVSTask;
		TaskId loadPSTask;

		if (!asyncParams->bUseAsync) {
			skyboxVSResource = manager->Load<ShaderResource>(vsParams);
			skyboxPSResource = manager->Load<ShaderResource>(psParams);
		} else {
			loadVSTask = manager->AsyncLoadDeferred<ShaderResource>(vsParams, 
				&skyboxVSResource);
			loadPSTask = manager->AsyncLoadDeferred<ShaderResource>(psParams, 
				&skyboxPSResource);
		}

		std::function<void()> buildPipeline = [=]() {
			auto skyboxVS = skyboxVSResource->GetShader();
			auto skyboxPS = skyboxPSResource->GetShader();

			auto anisotropyFactor = renderer->GetMaxAnisotropy();
			auto filterType = anisotropyFactor > 1 ? DG::FILTER_TYPE_ANISOTROPIC : DG::FILTER_TYPE_LINEAR;

			DG::SamplerDesc SamLinearClampDesc
			{
				filterType, filterType, filterType, 
				DG::TEXTURE_ADDRESS_CLAMP, DG::TEXTURE_ADDRESS_CLAMP, DG::TEXTURE_ADDRESS_CLAMP
			};

			SamLinearClampDesc.MaxAnisotropy = anisotropyFactor;

			DG::IPipelineState* result = nullptr;

			// Create Irradiance Pipeline
			DG::GraphicsPipelineStateCreateInfo PSOCreateInfo;
			DG::PipelineStateDesc&              PSODesc          = PSOCreateInfo.PSODesc;
			DG::GraphicsPipelineDesc&           GraphicsPipeline = PSOCreateInfo.GraphicsPipeline;

			PSODesc.Name         = "Skybox Pipeline";
			PSODesc.PipelineType = DG::PIPELINE_TYPE_GRAPHICS;

			GraphicsPipeline.NumRenderTargets             = 1;
			GraphicsPipeline.RTVFormats[0]                = renderer->GetIntermediateFramebufferFormat();
			GraphicsPipeline.PrimitiveTopology            = DG::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			GraphicsPipeline.RasterizerDesc.CullMode      = DG::CULL_MODE_NONE;
			GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
			GraphicsPipeline.DepthStencilDesc.DepthFunc   = DG::COMPARISON_FUNC_LESS_EQUAL;
			GraphicsPipeline.DSVFormat 					  = renderer->GetIntermediateDepthbufferFormat();

			// Number of MSAA samples
			GraphicsPipeline.SmplDesc.Count = (DG::Uint8)renderer->GetMSAASamples();

			GraphicsPipeline.InputLayout.NumElements = 0;

			PSOCreateInfo.pVS = skyboxVS;
			PSOCreateInfo.pPS = skyboxPS;

			PSODesc.ResourceLayout.DefaultVariableType = DG::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
			// clang-format off
			DG::ShaderResourceVariableDesc Vars[] = 
			{
				{DG::SHADER_TYPE_PIXEL, "mTexture", DG::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}
			};
			// clang-format on
			PSODesc.ResourceLayout.NumVariables = _countof(Vars);
			PSODesc.ResourceLayout.Variables    = Vars;

			// clang-format off
			DG::ImmutableSamplerDesc ImtblSamplers[] =
			{
				{DG::SHADER_TYPE_PIXEL, "mTexture_sampler", SamLinearClampDesc}
			};
			// clang-format on
			PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
			PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
			
			device->CreateGraphicsPipelineState(PSOCreateInfo, &result);
			result->GetStaticVariableByName(DG::SHADER_TYPE_VERTEX, "Globals")->Set(renderer->GetGlobalsBuffer());

			skyboxVSResource->Release();
			skyboxPSResource->Release();

			into->SetAll(
				result,
				std::vector<DG::LayoutElement>(),
				VertexAttributeLayout(),
				InstancingType::NONE);
		};

		if (!asyncParams->bUseAsync) {
			buildPipeline(); // Build pipeline on the current thread
			return TASK_NONE;
		} else {
			auto queue = asyncParams->mThreadPool->GetQueue();

			TaskId buildPipelineTask = queue.MakeTask([buildPipeline](const TaskParams& params) { 
				buildPipeline();
			}, postLoadBarrier, 0);

			// Schedule the loading of the build pipeline task
			queue.Dependencies(buildPipelineTask)
				.After(skyboxVSResource->GetLoadBarrier())
				.After(skyboxPSResource->GetLoadBarrier());

			postLoadBarrier->SetCallback(asyncParams->mCallback);

			// Create a deferred task to trigger the loading of the vertex and pixel shaders
			return queue.MakeTask([loadVSTask, loadPSTask](const TaskParams& params) {
				auto queue = params.mPool->GetQueue();

				// Load vertex and pixel shaders
				queue.Schedule(loadVSTask);
				queue.Schedule(loadPSTask);
			});
		}
	}
}