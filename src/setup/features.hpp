#pragma once

#include "external.hpp"
#include "setup/result.hpp"

class FeatureSetView {

	public:

		READONLY VkPhysicalDeviceFeatures vk_features;

	private:

		friend class FeatureSet;

		explicit FeatureSetView(const VkPhysicalDeviceFeatures& vk_features)
		: vk_features(vk_features) {}

	public:

		bool hasRobustBufferAccess() const {
			return vk_features.robustBufferAccess;
		}

		bool hasFullDrawIndexUint32() const {
			return vk_features.fullDrawIndexUint32;
		}

		bool hasImageCubeArray() const {
			return vk_features.imageCubeArray;
		}

		bool hasIndependentBlend() const {
			return vk_features.independentBlend;
		}

		bool hasGeometryShader() const {
			return vk_features.geometryShader;
		}

		bool hasTessellationShader() const {
			return vk_features.tessellationShader;
		}

		bool hasSampleRateShading() const {
			return vk_features.sampleRateShading;
		}

		bool hasDualSrcBlend() const {
			return vk_features.dualSrcBlend;
		}

		bool hasLogicOp() const {
			return vk_features.logicOp;
		}

		bool hasMultiDrawIndirect() const {
			return vk_features.multiDrawIndirect;
		}

		bool hasDrawIndirectFirstInstance() const {
			return vk_features.drawIndirectFirstInstance;
		}

		bool hasDepthClamp() const {
			return vk_features.depthClamp;
		}

		bool hasDepthBiasClamp() const {
			return vk_features.depthBiasClamp;
		}

		bool hasFillModeNonSolid() const {
			return vk_features.fillModeNonSolid;
		}

		bool hasDepthBounds() const {
			return vk_features.depthBounds;
		}

		bool hasWideLines() const {
			return vk_features.wideLines;
		}

		bool hasLargePoints() const {
			return vk_features.largePoints;
		}

		bool hasAlphaToOne() const {
			return vk_features.alphaToOne;
		}

		bool hasMultiViewport() const {
			return vk_features.multiViewport;
		}

		bool hasSamplerAnisotropy() const {
			return vk_features.samplerAnisotropy;
		}

		bool hasTextureCompressionETC2() const {
			return vk_features.textureCompressionETC2;
		}

		bool hasTextureCompressionASTC_LDR() const {
			return vk_features.textureCompressionASTC_LDR;
		}

		bool hasTextureCompressionBC() const {
			return vk_features.textureCompressionBC;
		}

		bool hasOcclusionQueryPrecise() const {
			return vk_features.occlusionQueryPrecise;
		}

		bool hasPipelineStatisticsQuery() const {
			return vk_features.pipelineStatisticsQuery;
		}

		bool hasVertexPipelineStoresAndAtomics() const {
			return vk_features.vertexPipelineStoresAndAtomics;
		}

		bool hasFragmentStoresAndAtomics() const {
			return vk_features.fragmentStoresAndAtomics;
		}

		bool hasShaderTessellationAndGeometryPointSize() const {
			return vk_features.shaderTessellationAndGeometryPointSize;
		}

		bool hasShaderImageGatherExtended() const {
			return vk_features.shaderImageGatherExtended;
		}

		bool hasShaderStorageImageExtendedFormats() const {
			return vk_features.shaderStorageImageExtendedFormats;
		}

		bool hasShaderStorageImageMultisample() const {
			return vk_features.shaderStorageImageMultisample;
		}

		bool hasShaderStorageImageReadWithoutFormat() const {
			return vk_features.shaderStorageImageReadWithoutFormat;
		}

		bool hasShaderStorageImageWriteWithoutFormat() const {
			return vk_features.shaderStorageImageWriteWithoutFormat;
		}

		bool hasShaderUniformBufferArrayDynamicIndexing() const {
			return vk_features.shaderUniformBufferArrayDynamicIndexing;
		}

		bool hasShaderSampledImageArrayDynamicIndexing() const {
			return vk_features.shaderSampledImageArrayDynamicIndexing;
		}

		bool hasShaderStorageBufferArrayDynamicIndexing() const {
			return vk_features.shaderStorageBufferArrayDynamicIndexing;
		}

		bool hasShaderStorageImageArrayDynamicIndexing() const {
			return vk_features.shaderStorageImageArrayDynamicIndexing;
		}

		bool hasShaderClipDistance() const {
			return vk_features.shaderClipDistance;
		}

		bool hasShaderCullDistance() const {
			return vk_features.shaderCullDistance;
		}

		bool hasShaderFloat64() const {
			return vk_features.shaderFloat64;
		}

		bool hasShaderInt64() const {
			return vk_features.shaderInt64;
		}

		bool hasShaderInt16() const {
			return vk_features.shaderInt16;
		}

		bool hasShaderResourceResidency() const {
			return vk_features.shaderResourceResidency;
		}

		bool hasShaderResourceMinLod() const {
			return vk_features.shaderResourceMinLod;
		}

		bool hasSparseBinding() const {
			return vk_features.sparseBinding;
		}

		bool hasSparseResidencyBuffer() const {
			return vk_features.sparseResidencyBuffer;
		}

		bool hasSparseResidencyImage2D() const {
			return vk_features.sparseResidencyImage2D;
		}

		bool hasSparseResidencyImage3D() const {
			return vk_features.sparseResidencyImage3D;
		}

		bool hasSparseResidency2Samples() const {
			return vk_features.sparseResidency2Samples;
		}

		bool hasSparseResidency4Samples() const {
			return vk_features.sparseResidency4Samples;
		}

		bool hasSparseResidency8Samples() const {
			return vk_features.sparseResidency8Samples;
		}

		bool hasSparseResidency16Samples() const {
			return vk_features.sparseResidency16Samples;
		}

		bool hasSparseResidencyAliased() const {
			return vk_features.sparseResidencyAliased;
		}

		bool hasVariableMultisampleRate() const {
			return vk_features.variableMultisampleRate;
		}

		bool hasInheritedQueries() const {
			return vk_features.inheritedQueries;
		}

};

class FeatureSet {

	private:

		VkPhysicalDeviceFeatures available;
		VkPhysicalDeviceFeatures selected;

	public:

		FeatureSet(const VkPhysicalDeviceFeatures& available)
		: available(available), selected({}) {}

		FeatureSetView view() const {
			return FeatureSetView {selected};
		}

	public:

		FeatureSet(FeatureSet&&) = default;
		FeatureSet(const FeatureSet&) = default;
		FeatureSet& operator=(FeatureSet&& other) = delete;

		void enableAll() {
			selected = available;
		}

		// the following part was auto generated with a python script

		Result<const char*> enableRobustBufferAccess() {
			if (!available.robustBufferAccess) {
				return {"Device feature 'robustBufferAccess' not available!", false};
			}

			selected.robustBufferAccess = true;
			return {"Device feature 'robustBufferAccess' available!", true};
		}

		Result<const char*> enableFullDrawIndexUint32() {
			if (!available.fullDrawIndexUint32) {
				return {"Device feature 'fullDrawIndexUint32' not available!", false};
			}

			selected.fullDrawIndexUint32 = true;
			return {"Device feature 'fullDrawIndexUint32' available!", true};
		}

		Result<const char*> enableImageCubeArray() {
			if (!available.imageCubeArray) {
				return {"Device feature 'imageCubeArray' not available!", false};
			}

			selected.imageCubeArray = true;
			return {"Device feature 'imageCubeArray' available!", true};
		}

		Result<const char*> enableIndependentBlend() {
			if (!available.independentBlend) {
				return {"Device feature 'independentBlend' not available!", false};
			}

			selected.independentBlend = true;
			return {"Device feature 'independentBlend' available!", true};
		}

		Result<const char*> enableGeometryShader() {
			if (!available.geometryShader) {
				return {"Device feature 'geometryShader' not available!", false};
			}

			selected.geometryShader = true;
			return {"Device feature 'geometryShader' available!", true};
		}

		Result<const char*> enableTessellationShader() {
			if (!available.tessellationShader) {
				return {"Device feature 'tessellationShader' not available!", false};
			}

			selected.tessellationShader = true;
			return {"Device feature 'tessellationShader' available!", true};
		}

		Result<const char*> enableSampleRateShading() {
			if (!available.sampleRateShading) {
				return {"Device feature 'sampleRateShading' not available!", false};
			}

			selected.sampleRateShading = true;
			return {"Device feature 'sampleRateShading' available!", true};
		}

		Result<const char*> enableDualSrcBlend() {
			if (!available.dualSrcBlend) {
				return {"Device feature 'dualSrcBlend' not available!", false};
			}

			selected.dualSrcBlend = true;
			return {"Device feature 'dualSrcBlend' available!", true};
		}

		Result<const char*> enableLogicOp() {
			if (!available.logicOp) {
				return {"Device feature 'logicOp' not available!", false};
			}

			selected.logicOp = true;
			return {"Device feature 'logicOp' available!", true};
		}

		Result<const char*> enableMultiDrawIndirect() {
			if (!available.multiDrawIndirect) {
				return {"Device feature 'multiDrawIndirect' not available!", false};
			}

			selected.multiDrawIndirect = true;
			return {"Device feature 'multiDrawIndirect' available!", true};
		}

		Result<const char*> enableDrawIndirectFirstInstance() {
			if (!available.drawIndirectFirstInstance) {
				return {"Device feature 'drawIndirectFirstInstance' not available!", false};
			}

			selected.drawIndirectFirstInstance = true;
			return {"Device feature 'drawIndirectFirstInstance' available!", true};
		}

		Result<const char*> enableDepthClamp() {
			if (!available.depthClamp) {
				return {"Device feature 'depthClamp' not available!", false};
			}

			selected.depthClamp = true;
			return {"Device feature 'depthClamp' available!", true};
		}

		Result<const char*> enableDepthBiasClamp() {
			if (!available.depthBiasClamp) {
				return {"Device feature 'depthBiasClamp' not available!", false};
			}

			selected.depthBiasClamp = true;
			return {"Device feature 'depthBiasClamp' available!", true};
		}

		Result<const char*> enableFillModeNonSolid() {
			if (!available.fillModeNonSolid) {
				return {"Device feature 'fillModeNonSolid' not available!", false};
			}

			selected.fillModeNonSolid = true;
			return {"Device feature 'fillModeNonSolid' available!", true};
		}

		Result<const char*> enableDepthBounds() {
			if (!available.depthBounds) {
				return {"Device feature 'depthBounds' not available!", false};
			}

			selected.depthBounds = true;
			return {"Device feature 'depthBounds' available!", true};
		}

		Result<const char*> enableWideLines() {
			if (!available.wideLines) {
				return {"Device feature 'wideLines' not available!", false};
			}

			selected.wideLines = true;
			return {"Device feature 'wideLines' available!", true};
		}

		Result<const char*> enableLargePoints() {
			if (!available.largePoints) {
				return {"Device feature 'largePoints' not available!", false};
			}

			selected.largePoints = true;
			return {"Device feature 'largePoints' available!", true};
		}

		Result<const char*> enableAlphaToOne() {
			if (!available.alphaToOne) {
				return {"Device feature 'alphaToOne' not available!", false};
			}

			selected.alphaToOne = true;
			return {"Device feature 'alphaToOne' available!", true};
		}

		Result<const char*> enableMultiViewport() {
			if (!available.multiViewport) {
				return {"Device feature 'multiViewport' not available!", false};
			}

			selected.multiViewport = true;
			return {"Device feature 'multiViewport' available!", true};
		}

		Result<const char*> enableSamplerAnisotropy() {
			if (!available.samplerAnisotropy) {
				return {"Device feature 'samplerAnisotropy' not available!", false};
			}

			selected.samplerAnisotropy = true;
			return {"Device feature 'samplerAnisotropy' available!", true};
		}

		Result<const char*> enableTextureCompressionETC2() {
			if (!available.textureCompressionETC2) {
				return {"Device feature 'textureCompressionETC2' not available!", false};
			}

			selected.textureCompressionETC2 = true;
			return {"Device feature 'textureCompressionETC2' available!", true};
		}

		Result<const char*> enableTextureCompressionASTC_LDR() {
			if (!available.textureCompressionASTC_LDR) {
				return {"Device feature 'textureCompressionASTC_LDR' not available!", false};
			}

			selected.textureCompressionASTC_LDR = true;
			return {"Device feature 'textureCompressionASTC_LDR' available!", true};
		}

		Result<const char*> enableTextureCompressionBC() {
			if (!available.textureCompressionBC) {
				return {"Device feature 'textureCompressionBC' not available!", false};
			}

			selected.textureCompressionBC = true;
			return {"Device feature 'textureCompressionBC' available!", true};
		}

		Result<const char*> enableOcclusionQueryPrecise() {
			if (!available.occlusionQueryPrecise) {
				return {"Device feature 'occlusionQueryPrecise' not available!", false};
			}

			selected.occlusionQueryPrecise = true;
			return {"Device feature 'occlusionQueryPrecise' available!", true};
		}

		Result<const char*> enablePipelineStatisticsQuery() {
			if (!available.pipelineStatisticsQuery) {
				return {"Device feature 'pipelineStatisticsQuery' not available!", false};
			}

			selected.pipelineStatisticsQuery = true;
			return {"Device feature 'pipelineStatisticsQuery' available!", true};
		}

		Result<const char*> enableVertexPipelineStoresAndAtomics() {
			if (!available.vertexPipelineStoresAndAtomics) {
				return {"Device feature 'vertexPipelineStoresAndAtomics' not available!", false};
			}

			selected.vertexPipelineStoresAndAtomics = true;
			return {"Device feature 'vertexPipelineStoresAndAtomics' available!", true};
		}

		Result<const char*> enableFragmentStoresAndAtomics() {
			if (!available.fragmentStoresAndAtomics) {
				return {"Device feature 'fragmentStoresAndAtomics' not available!", false};
			}

			selected.fragmentStoresAndAtomics = true;
			return {"Device feature 'fragmentStoresAndAtomics' available!", true};
		}

		Result<const char*> enableShaderTessellationAndGeometryPointSize() {
			if (!available.shaderTessellationAndGeometryPointSize) {
				return {"Device feature 'shaderTessellationAndGeometryPointSize' not available!", false};
			}

			selected.shaderTessellationAndGeometryPointSize = true;
			return {"Device feature 'shaderTessellationAndGeometryPointSize' available!", true};
		}

		Result<const char*> enableShaderImageGatherExtended() {
			if (!available.shaderImageGatherExtended) {
				return {"Device feature 'shaderImageGatherExtended' not available!", false};
			}

			selected.shaderImageGatherExtended = true;
			return {"Device feature 'shaderImageGatherExtended' available!", true};
		}

		Result<const char*> enableShaderStorageImageExtendedFormats() {
			if (!available.shaderStorageImageExtendedFormats) {
				return {"Device feature 'shaderStorageImageExtendedFormats' not available!", false};
			}

			selected.shaderStorageImageExtendedFormats = true;
			return {"Device feature 'shaderStorageImageExtendedFormats' available!", true};
		}

		Result<const char*> enableShaderStorageImageMultisample() {
			if (!available.shaderStorageImageMultisample) {
				return {"Device feature 'shaderStorageImageMultisample' not available!", false};
			}

			selected.shaderStorageImageMultisample = true;
			return {"Device feature 'shaderStorageImageMultisample' available!", true};
		}

		Result<const char*> enableShaderStorageImageReadWithoutFormat() {
			if (!available.shaderStorageImageReadWithoutFormat) {
				return {"Device feature 'shaderStorageImageReadWithoutFormat' not available!", false};
			}

			selected.shaderStorageImageReadWithoutFormat = true;
			return {"Device feature 'shaderStorageImageReadWithoutFormat' available!", true};
		}

		Result<const char*> enableShaderStorageImageWriteWithoutFormat() {
			if (!available.shaderStorageImageWriteWithoutFormat) {
				return {"Device feature 'shaderStorageImageWriteWithoutFormat' not available!", false};
			}

			selected.shaderStorageImageWriteWithoutFormat = true;
			return {"Device feature 'shaderStorageImageWriteWithoutFormat' available!", true};
		}

		Result<const char*> enableShaderUniformBufferArrayDynamicIndexing() {
			if (!available.shaderUniformBufferArrayDynamicIndexing) {
				return {"Device feature 'shaderUniformBufferArrayDynamicIndexing' not available!", false};
			}

			selected.shaderUniformBufferArrayDynamicIndexing = true;
			return {"Device feature 'shaderUniformBufferArrayDynamicIndexing' available!", true};
		}

		Result<const char*> enableShaderSampledImageArrayDynamicIndexing() {
			if (!available.shaderSampledImageArrayDynamicIndexing) {
				return {"Device feature 'shaderSampledImageArrayDynamicIndexing' not available!", false};
			}

			selected.shaderSampledImageArrayDynamicIndexing = true;
			return {"Device feature 'shaderSampledImageArrayDynamicIndexing' available!", true};
		}

		Result<const char*> enableShaderStorageBufferArrayDynamicIndexing() {
			if (!available.shaderStorageBufferArrayDynamicIndexing) {
				return {"Device feature 'shaderStorageBufferArrayDynamicIndexing' not available!", false};
			}

			selected.shaderStorageBufferArrayDynamicIndexing = true;
			return {"Device feature 'shaderStorageBufferArrayDynamicIndexing' available!", true};
		}

		Result<const char*> enableShaderStorageImageArrayDynamicIndexing() {
			if (!available.shaderStorageImageArrayDynamicIndexing) {
				return {"Device feature 'shaderStorageImageArrayDynamicIndexing' not available!", false};
			}

			selected.shaderStorageImageArrayDynamicIndexing = true;
			return {"Device feature 'shaderStorageImageArrayDynamicIndexing' available!", true};
		}

		Result<const char*> enableShaderClipDistance() {
			if (!available.shaderClipDistance) {
				return {"Device feature 'shaderClipDistance' not available!", false};
			}

			selected.shaderClipDistance = true;
			return {"Device feature 'shaderClipDistance' available!", true};
		}

		Result<const char*> enableShaderCullDistance() {
			if (!available.shaderCullDistance) {
				return {"Device feature 'shaderCullDistance' not available!", false};
			}

			selected.shaderCullDistance = true;
			return {"Device feature 'shaderCullDistance' available!", true};
		}

		Result<const char*> enableShaderFloat64() {
			if (!available.shaderFloat64) {
				return {"Device feature 'shaderFloat64' not available!", false};
			}

			selected.shaderFloat64 = true;
			return {"Device feature 'shaderFloat64' available!", true};
		}

		Result<const char*> enableShaderInt64() {
			if (!available.shaderInt64) {
				return {"Device feature 'shaderInt64' not available!", false};
			}

			selected.shaderInt64 = true;
			return {"Device feature 'shaderInt64' available!", true};
		}

		Result<const char*> enableShaderInt16() {
			if (!available.shaderInt16) {
				return {"Device feature 'shaderInt16' not available!", false};
			}

			selected.shaderInt16 = true;
			return {"Device feature 'shaderInt16' available!", true};
		}

		Result<const char*> enableShaderResourceResidency() {
			if (!available.shaderResourceResidency) {
				return {"Device feature 'shaderResourceResidency' not available!", false};
			}

			selected.shaderResourceResidency = true;
			return {"Device feature 'shaderResourceResidency' available!", true};
		}

		Result<const char*> enableShaderResourceMinLod() {
			if (!available.shaderResourceMinLod) {
				return {"Device feature 'shaderResourceMinLod' not available!", false};
			}

			selected.shaderResourceMinLod = true;
			return {"Device feature 'shaderResourceMinLod' available!", true};
		}

		Result<const char*> enableSparseBinding() {
			if (!available.sparseBinding) {
				return {"Device feature 'sparseBinding' not available!", false};
			}

			selected.sparseBinding = true;
			return {"Device feature 'sparseBinding' available!", true};
		}

		Result<const char*> enableSparseResidencyBuffer() {
			if (!available.sparseResidencyBuffer) {
				return {"Device feature 'sparseResidencyBuffer' not available!", false};
			}

			selected.sparseResidencyBuffer = true;
			return {"Device feature 'sparseResidencyBuffer' available!", true};
		}

		Result<const char*> enableSparseResidencyImage2D() {
			if (!available.sparseResidencyImage2D) {
				return {"Device feature 'sparseResidencyImage2D' not available!", false};
			}

			selected.sparseResidencyImage2D = true;
			return {"Device feature 'sparseResidencyImage2D' available!", true};
		}

		Result<const char*> enableSparseResidencyImage3D() {
			if (!available.sparseResidencyImage3D) {
				return {"Device feature 'sparseResidencyImage3D' not available!", false};
			}

			selected.sparseResidencyImage3D = true;
			return {"Device feature 'sparseResidencyImage3D' available!", true};
		}

		Result<const char*> enableSparseResidency2Samples() {
			if (!available.sparseResidency2Samples) {
				return {"Device feature 'sparseResidency2Samples' not available!", false};
			}

			selected.sparseResidency2Samples = true;
			return {"Device feature 'sparseResidency2Samples' available!", true};
		}

		Result<const char*> enableSparseResidency4Samples() {
			if (!available.sparseResidency4Samples) {
				return {"Device feature 'sparseResidency4Samples' not available!", false};
			}

			selected.sparseResidency4Samples = true;
			return {"Device feature 'sparseResidency4Samples' available!", true};
		}

		Result<const char*> enableSparseResidency8Samples() {
			if (!available.sparseResidency8Samples) {
				return {"Device feature 'sparseResidency8Samples' not available!", false};
			}

			selected.sparseResidency8Samples = true;
			return {"Device feature 'sparseResidency8Samples' available!", true};
		}

		Result<const char*> enableSparseResidency16Samples() {
			if (!available.sparseResidency16Samples) {
				return {"Device feature 'sparseResidency16Samples' not available!", false};
			}

			selected.sparseResidency16Samples = true;
			return {"Device feature 'sparseResidency16Samples' available!", true};
		}

		Result<const char*> enableSparseResidencyAliased() {
			if (!available.sparseResidencyAliased) {
				return {"Device feature 'sparseResidencyAliased' not available!", false};
			}

			selected.sparseResidencyAliased = true;
			return {"Device feature 'sparseResidencyAliased' available!", true};
		}

		Result<const char*> enableVariableMultisampleRate() {
			if (!available.variableMultisampleRate) {
				return {"Device feature 'variableMultisampleRate' not available!", false};
			}

			selected.variableMultisampleRate = true;
			return {"Device feature 'variableMultisampleRate' available!", true};
		}

		Result<const char*> enableInheritedQueries() {
			if (!available.inheritedQueries) {
				return {"Device feature 'inheritedQueries' not available!", false};
			}

			selected.inheritedQueries = true;
			return {"Device feature 'inheritedQueries' available!", true};
		}

};
