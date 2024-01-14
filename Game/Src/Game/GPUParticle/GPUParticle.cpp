#include "GPUParticle.h"

#include <d3dx12.h>

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/Graphics/SamplerManager.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Input/Input.h"

//const UINT GPUParticle::kNumThread = 524288;
const UINT GPUParticle::kNumThread = 128;
const UINT GPUParticle::CommandSizePerFrame = kNumThread * sizeof(IndirectCommand);
const UINT GPUParticle::CommandBufferCounterOffset = AlignForUavCounter(GPUParticle::CommandSizePerFrame);

GPUParticle::GPUParticle() {
	graphicsPipelineState_ = std::make_unique<PipelineState>();
	graphicsRootSignature_ = std::make_unique<RootSignature>();

	spawnComputePipelineState_ = std::make_unique<PipelineState>();
	spawnComputeRootSignature_ = std::make_unique<RootSignature>();

	updateComputePipelineState_ = std::make_unique<PipelineState>();
	updateComputeRootSignature_ = std::make_unique<RootSignature>();

	InitializeGraphics();

	InitializeParticleArea();

	InitializeSpawnParticle();

	InitializeUpdateParticle();

	InitializeBall();

	ballModelHandle_ = ModelManager::GetInstance()->Load("Game/Resources/Models/Ball");
	gpuParticleModelHandle_ = ModelManager::GetInstance()->Load("Game/Resources/Models/GPUParticle");


	for (auto& worldTransform : ballWorldTransform_) {
		worldTransform.Initialize();
	}

	worldTransform_.Initialize();
	worldTransform_.translation_.z = 30.0f;
	worldTransform_.UpdateMatrix();
	shotTime = 0;
}

void GPUParticle::Initialize() {

}

void GPUParticle::Update(ViewProjection* viewProjection) {
	Vector3 vector{};
	Vector3 cameraMove{};
	// ゲームパットの状態を得る変数
	XINPUT_STATE joyState{};
	// ゲームパットの状況取得
	// 入力がなかったら何もしない
	if (Input::GetInstance()->IsControllerConnected()) {
		Input::GetInstance()->GetJoystickState(0, joyState);
		const float kMargin = 0.7f;
		// 移動量
		Vector3 move = {
			static_cast<float>(joyState.Gamepad.sThumbLX),
			0.0f,
			static_cast<float>(joyState.Gamepad.sThumbLY),
		};
		cameraMove = {
			-static_cast<float>(joyState.Gamepad.sThumbRY),
			static_cast<float>(joyState.Gamepad.sThumbRX),
			0.0f,
		};
		if (move.Length() > kMargin) {
			vector = Normalize(move);
		}
		if (cameraMove.Length() > 0.0f) {
			cameraMove = Normalize(cameraMove);
		}
	}
	if (Input::GetInstance()->PushKey(DIK_W)) {
		vector.z = 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_A)) {
		vector.x = -1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		vector.z = -1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_D)) {
		vector.x = 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_R)) {
		auto commandContext = RenderManager::GetInstance()->GetCommandContext();
		commandContext.SetPipelineState(*spawnComputePipelineState_);
		commandContext.SetComputeRootSignature(*spawnComputeRootSignature_);

		commandContext.TransitionResource(rwStructuredBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(particleAreaBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.SetComputeUAV(0, rwStructuredBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(1, particleAreaBuffer_->GetGPUVirtualAddress());

		commandContext.Dispatch(static_cast<UINT>(ceil(kNumThread / float(ComputeThreadBlockSize))), 1, 1);

		commandContext.Close();

		CommandQueue& commandQueue = GraphicsCore::GetInstance()->GetCommandQueue();
		commandQueue.Execute(commandContext);
		commandQueue.Signal();
		commandQueue.WaitForGPU();
		commandContext.Reset();
	}
	if (Input::GetInstance()->PushKey(DIK_UPARROW)) {
		cameraMove.x = -1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_DOWNARROW)) {
		cameraMove.x = 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_LEFTARROW)) {
		cameraMove.y = -1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_RIGHTARROW)) {
		cameraMove.y = 1.0f;
	}
	Matrix4x4 rotate = MakeRotateYMatrix(viewProjection->rotation_.y);
	vector = TransformNormal(vector * 0.2f, rotate);
	viewProjection->translation_ += vector;
	viewProjection->rotation_ += cameraMove * 0.05f;
	viewProjection->UpdateMatrix();

	if (shotTime > 0) {
		shotTime--;
	}
	else {
		shotTime = 0;
	}

	if (shotTime == 0 &&
		(Input::GetInstance()->PushKey(DIK_SPACE) ||
			(Input::GetInstance()->GetJoystickState(0, joyState) &&
				(joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A)))) {
		for (size_t i = 0; i < kMaxBall; i++) {
			if (!ballData_.at(i).isAlive) {
				ballData_.at(i).isAlive = true;
				rotate = MakeRotateXYZMatrix(viewProjection->rotation_);
				Vector3 ballVector = Normalize(TransformNormal({ 0.0f,0.0f,1.0f }, rotate));
				ballData_.at(i).position = viewProjection->translation_ + ballVector * 10.0f;
				ballData_.at(i).velocity = ballVector;
				ballData_.at(i).size = 1.0f;
				ballData_.at(i).aliveTime = kAliveTime;
				break;
			}
		}
		shotTime = kShotCoolTime;
	}
	ballCount_->ballCount = 0;
	for (size_t i = 0; i < kMaxBall; i++) {
		if (ballData_.at(i).isAlive) {
			ballData_.at(i).position += ballData_.at(i).velocity;
			ballData_.at(i).aliveTime--;
			if (ballData_.at(i).aliveTime >= 0) {
				ball_[ballCount_->ballCount].position = ballData_.at(i).position;
				ball_[ballCount_->ballCount].size = ballData_.at(i).size;
				ballWorldTransform_.at(i).translation_ = ball_[ballCount_->ballCount].position;
				ballWorldTransform_.at(i).scale_.x = ball_[ballCount_->ballCount].size;
				ballWorldTransform_.at(i).scale_.y = ball_[ballCount_->ballCount].size;
				ballWorldTransform_.at(i).scale_.z = ball_[ballCount_->ballCount].size;
				ballWorldTransform_.at(i).UpdateMatrix();
				ballCount_->ballCount++;
			}
			else {
				ballData_.at(i).isAlive = false;
			}
		}
	}


	ballBuffer_.Copy(ball_, sizeof(BallBufferData) * kMaxBall);
	ballCountBuffer_.Copy(ballCount_, sizeof(BallCount));

	auto commandContext = RenderManager::GetInstance()->GetCommandContext();
	commandContext.SetPipelineState(*updateComputePipelineState_);
	commandContext.SetComputeRootSignature(*updateComputeRootSignature_);

	// リセット
	commandContext.CopyBufferRegion(processedCommandBuffers_, CommandBufferCounterOffset, processedCommandBufferCounterReset_, 0, sizeof(UINT));

	commandContext.TransitionResource(rwStructuredBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(processedCommandBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.SetComputeUAV(0, rwStructuredBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, commandHandle_);
	commandContext.SetComputeDescriptorTable(2, processedCommandsHandle_);
	commandContext.SetComputeConstantBuffer(3, updateConstantBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(4, ballBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(5, ballCountBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(6, particleAreaBuffer_->GetGPUVirtualAddress());

	commandContext.Dispatch(static_cast<UINT>(ceil(kNumThread / float(ComputeThreadBlockSize))), 1, 1);

	commandContext.Close();

	CommandQueue& commandQueue = GraphicsCore::GetInstance()->GetCommandQueue();
	commandQueue.Execute(commandContext);
	commandQueue.Signal();
	commandQueue.WaitForGPU();
	commandContext.Reset();
}

void GPUParticle::Render(const ViewProjection& viewProjection) {
	auto graphics = GraphicsCore::GetInstance();
	auto commandContext = RenderManager::GetInstance()->GetCommandContext();

	for (size_t i = 0; i < kMaxBall; i++) {
		if (ballData_.at(i).isAlive) {
			ModelManager::GetInstance()->Draw(ballWorldTransform_.at(i), viewProjection, ballModelHandle_, commandContext);
		}
	}
	ModelManager::GetInstance()->Draw(worldTransform_, viewProjection, gpuParticleModelHandle_, commandContext);
	commandContext.SetPipelineState(*graphicsPipelineState_);
	commandContext.SetGraphicsRootSignature(*graphicsRootSignature_);

	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandContext.SetGraphicsConstantBuffer(1, viewProjection.constBuff_->GetGPUVirtualAddress());
	commandContext.SetGraphicsDescriptorTable(2, TextureManager::GetInstance()->GetTexture(ModelManager::GetInstance()->GetModel(gpuParticleModelHandle_).GetTextureHandle()).GetSRV());
	commandContext.SetGraphicsDescriptorTable(3, SamplerManager::Anisotropic);
	commandContext.SetVertexBuffer(0, vbView_);
	commandContext.SetIndexBuffer(ibView_);
	commandContext.TransitionResource(processedCommandBuffers_, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	commandContext.ExecuteIndirect(
		commandSignature_.Get(),
		kNumThread,
		processedCommandBuffers_,
		0,
		processedCommandBuffers_,
		CommandBufferCounterOffset
	);

}

void GPUParticle::InitializeBall() {
	auto graphics = GraphicsCore::GetInstance();
	ballBuffer_.Create(L"BallBufferData Buffer", sizeof(BallBufferData) * kMaxBall);
	ball_ = new BallBufferData();
	for (int i = 0; i < kMaxBall; i++) {
		ballData_.at(i).position = { 0.0f,0.0f,0.0f };
		ballData_.at(i).isAlive = false;
		ballData_.at(i).aliveTime = kAliveTime;
		ballData_.at(i).size = 1.0f;
	}
	ballBuffer_.Copy(ball_, sizeof(BallBufferData) * kMaxBall);

	// シェーダーリソースビュー
	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	desc.Buffer.NumElements = kMaxBall;
	desc.Buffer.StructureByteStride = sizeof(BallBufferData);
	ballBufferHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	ballCountBuffer_.Create(L"BallCountBuffer", sizeof(BallCount));
	ballCount_ = new BallCount();
	ballCount_->ballCount = 10;
	ballCountBuffer_.Copy(ballCount_, sizeof(BallCount));
}

void GPUParticle::InitializeSpawnParticle() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();

	// RWStructuredBuffer
	{
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(
			UINT64(sizeof(Particle) * kNumThread),
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(rwStructuredBuffer_.GetAddressOf())
		);
	}
	// 初期化用コンピュートルートシグネイチャ
	{
		// コマンド用
		CD3DX12_ROOT_PARAMETER rootParameters[2]{};
		rootParameters[0].InitAsUnorderedAccessView(0);
		rootParameters[1].InitAsConstantBufferView(0);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		spawnComputeRootSignature_->Create(L"GPUParticle SpawnRootSignature", desc);

	}
	// 初期化用コンピュートパイプライン
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *spawnComputeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Game/Resources/Shaders/Spawn.CS.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		spawnComputePipelineState_->Create(L"GPUParticle SpawnCPSO", desc);
	}
	// 初期化
	{
		auto commandContext = RenderManager::GetInstance()->GetCommandContext();
		commandContext.SetPipelineState(*spawnComputePipelineState_);
		commandContext.SetComputeRootSignature(*spawnComputeRootSignature_);

		commandContext.TransitionResource(rwStructuredBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(particleAreaBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.SetComputeUAV(0, rwStructuredBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(1, particleAreaBuffer_->GetGPUVirtualAddress());

		commandContext.Dispatch(static_cast<UINT>(ceil(kNumThread / float(ComputeThreadBlockSize))), 1, 1);

		commandContext.Close();

		CommandQueue& commandQueue = GraphicsCore::GetInstance()->GetCommandQueue();
		commandQueue.Execute(commandContext);
		commandQueue.Signal();
		commandQueue.WaitForGPU();
		commandContext.Reset();
	}
}

void GPUParticle::InitializeUpdateParticle() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();
	// アップデート用コンピュートシグネイチャ
	{
		// コマンド用
		CD3DX12_DESCRIPTOR_RANGE commandRanges[1]{};
		commandRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

		// AppendStructuredBuffer用（カウンター付きUAVの場合このように宣言）
		CD3DX12_DESCRIPTOR_RANGE appendRanges[1]{};
		appendRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[7]{};
		rootParameters[0].InitAsUnorderedAccessView(0);
		rootParameters[1].InitAsDescriptorTable(_countof(commandRanges), commandRanges);
		rootParameters[2].InitAsDescriptorTable(_countof(appendRanges), appendRanges);
		rootParameters[3].InitAsConstantBufferView(0);
		rootParameters[4].InitAsShaderResourceView(1);
		rootParameters[5].InitAsConstantBufferView(1);
		rootParameters[6].InitAsConstantBufferView(2);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		updateComputeRootSignature_->Create(L"GPUParticle UpdateRootSignature", desc);
	}
	// アップデート用コンピュートパイプライン
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *updateComputeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Game/Resources/Shaders/Update.CS.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		updateComputePipelineState_->Create(L"GPUParticle UpdateCPSO", desc);
	}
	// UAVの生成
	{
		uavHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Buffer.NumElements = kNumThread;
		desc.Buffer.StructureByteStride = sizeof(Particle);
		device->CreateUnorderedAccessView(
			rwStructuredBuffer_,
			nullptr,
			&desc,
			uavHandle_
		);
	}
	// 定数バッファ
	{
		updateConstantBuffer_.Create(L"GPUParticle UpdateConstantBuffer", sizeof(ParticleInfo));
		particleInfo_ = new ParticleInfo();
		particleInfo_->speed = 0.05f;
		updateConstantBuffer_.Copy(particleInfo_, sizeof(ParticleInfo));
	}
	// コマンドシグネイチャー
	{
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
		argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		argumentDescs[0].ShaderResourceView.RootParameterIndex = 0;
		argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc{};
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = sizeof(IndirectCommand);
		auto result = device->CreateCommandSignature(&commandSignatureDesc, *graphicsRootSignature_, IID_PPV_ARGS(&commandSignature_));
		assert(SUCCEEDED(result));
	}
	// IndirectCommandBuffer
	{
		std::vector<IndirectCommand> commands;
		commands.resize(kNumThread);
		// Default
		D3D12_RESOURCE_DESC commandBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(CommandSizePerFrame);
		D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&commandBufferDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(commandBuffer_.GetAddressOf()));
		commandBuffer_->SetName(L"commandBuffer");
		// コピー用
		UploadBuffer commandBufferUpload;
		commandBufferUpload.Create(L"commandBufferUpload", CommandSizePerFrame);

		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = rwStructuredBuffer_.GetGPUVirtualAddress();

		for (UINT commandIndex = 0; commandIndex < kNumThread; ++commandIndex) {
			commands[commandIndex].srv = gpuAddress;

			commands[commandIndex].drawIndex.IndexCountPerInstance = UINT(indices_.size());
			commands[commandIndex].drawIndex.InstanceCount = 1;
			commands[commandIndex].drawIndex.StartIndexLocation = 0;
			commands[commandIndex].drawIndex.StartInstanceLocation = 0;
			commands[commandIndex].drawIndex.BaseVertexLocation = 0;
		}
		commandBufferUpload.Copy(commands.data(), CommandSizePerFrame);

		auto& commandContext = RenderManager::GetInstance()->GetCommandContext();
		commandContext.CopyBuffer(commandBuffer_, commandBufferUpload);

		// コピー
		{
			commandContext.Close();
			CommandQueue& commandQueue = GraphicsCore::GetInstance()->GetCommandQueue();
			commandQueue.Execute(commandContext);
			commandQueue.Signal();
			commandQueue.WaitForGPU();
			commandContext.Reset();
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.NumElements = kNumThread;
		srvDesc.Buffer.StructureByteStride = sizeof(IndirectCommand);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		commandHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(
			commandBuffer_,
			&srvDesc,
			commandHandle_
		);
		// 計算結果を積み込むよう
		commandBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(CommandBufferCounterOffset + sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&commandBufferDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(processedCommandBuffers_.GetAddressOf())
		);

		processedCommandsHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = kNumThread;
		uavDesc.Buffer.StructureByteStride = sizeof(IndirectCommand);
		uavDesc.Buffer.CounterOffsetInBytes = CommandBufferCounterOffset;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		device->CreateUnorderedAccessView(
			processedCommandBuffers_,
			processedCommandBuffers_,
			&uavDesc,
			processedCommandsHandle_);

		// リセット用
		commandBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT));
		heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&commandBufferDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(processedCommandBufferCounterReset_.GetAddressOf())
		);

		UINT8* pMappedCounterReset = nullptr;
		CD3DX12_RANGE readRange(0, 0);
		processedCommandBufferCounterReset_->Map(0, &readRange, reinterpret_cast<void**>(&pMappedCounterReset));
		ZeroMemory(pMappedCounterReset, sizeof(UINT));
		processedCommandBufferCounterReset_->Unmap(0, nullptr);

	}

}

void GPUParticle::InitializeGraphics() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();
	// グラフィックスルートシグネイチャ
	{
		CD3DX12_DESCRIPTOR_RANGE structuredBuffer[1]{};
		structuredBuffer[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE range[1]{};
		range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		CD3DX12_DESCRIPTOR_RANGE samplerRanges[1]{};
		samplerRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[4]{};
		rootParameters[0].InitAsShaderResourceView(0);
		rootParameters[1].InitAsConstantBufferView(0);
		rootParameters[2].InitAsDescriptorTable(_countof(range), range);
		rootParameters[3].InitAsDescriptorTable(_countof(samplerRanges), samplerRanges);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		graphicsRootSignature_->Create(L"GPUParticle RootSignature", desc);
	}
	// グラフィックスパイプライン
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

		desc.pRootSignature = *graphicsRootSignature_;

		D3D12_INPUT_ELEMENT_DESC inputElements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = inputElements;
		inputLayoutDesc.NumElements = _countof(inputElements);
		desc.InputLayout = inputLayoutDesc;

		auto vs = ShaderCompiler::Compile(L"Game/Resources/Shaders/GPUParticle.VS.hlsl", L"vs_6_0");
		auto ps = ShaderCompiler::Compile(L"Game/Resources/Shaders/GPUParticle.PS.hlsl", L"ps_6_0");
		desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
		desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
		desc.BlendState = Helper::BlendAdditive;
		desc.DepthStencilState = Helper::DepthStateDisabled;
		desc.RasterizerState = Helper::RasterizerDefault;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = RenderManager::GetInstance()->GetRenderTargetFormat();
		desc.DSVFormat = RenderManager::GetInstance()->GetDepthFormat();
		desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.SampleDesc.Count = 1;
		graphicsPipelineState_->Create(L"GPUParticle PSO", desc);
	}
	// シェーダーリソース用
	{
		rwStructuredBufferHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		desc.Buffer.NumElements = kNumThread;
		desc.Buffer.StructureByteStride = sizeof(Particle);
		device->CreateShaderResourceView(rwStructuredBuffer_, &desc, rwStructuredBufferHandle_);
	}
	// 頂点バッファ
	{
		vertices_ = {
			// 前
			{ { -0.5f, -0.5f, +0.0f },{0.0f,1.0f} }, // 左下
			{ { -0.5f, +0.5f, +0.0f },{0.0f,0.0f} }, // 左上
			{ { +0.5f, -0.5f, +0.0f },{1.0f,1.0f} }, // 右下
			{ { +0.5f, +0.5f, +0.0f },{1.0f,0.0f} }, // 右上
		};
		//vertices_ = {
		//	// 前
		//	{ { -0.5f, -0.5f, +0.0f },{0.0f,0.0f} }, // 左
		//	{ { +0.0f, +0.5f, +0.0f },{0.5f,0.5f} }, // 真ん中
		//	{ { +0.5f, -0.5f, +0.0f },{1.0f,0.0f} }, // 右
		//};

		size_t sizeIB = sizeof(Vertex) * vertices_.size();

		vertexBuffer_.Create(L"vertexBuffer", sizeIB);

		vertexBuffer_.Copy(vertices_.data(), sizeIB);

		vbView_.BufferLocation = vertexBuffer_.GetGPUVirtualAddress();
		vbView_.SizeInBytes = UINT(sizeIB);
		vbView_.StrideInBytes = sizeof(Vertex);
	}
	// インデックスバッファ
	{
		indices_ = {
		0, 1, 3,
		2, 0, 3,
		};

		size_t sizeIB = sizeof(uint16_t) * indices_.size();

		indexBuffer_.Create(L"indexBuffer", sizeIB);

		indexBuffer_.Copy(indices_.data(), sizeIB);

		// インデックスバッファビューの作成
		ibView_.BufferLocation = indexBuffer_.GetGPUVirtualAddress();
		ibView_.Format = DXGI_FORMAT_R16_UINT;
		ibView_.SizeInBytes = UINT(sizeIB);
	}
}


void GPUParticle::InitializeParticleArea() {
	particleAreaBuffer_.Create(L"GPUParticle ParticleAreaBuffer", sizeof(ParticleArea));
	particleArea_ = new ParticleArea();
	particleArea_->min = { -5.0f,-5.0f,-5.0f };
	particleArea_->max = { +5.0f,+5.0f,+5.0f };
	particleAreaBuffer_.Copy(particleArea_, sizeof(ParticleArea));
}
