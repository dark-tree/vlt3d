#pragma once

#include "external.hpp"
#include "setup/result.hpp"

#define FEATURE_ENABLE_IF(device_builder, feature_name) if (device_builder.supported_features.feature_name) device_builder.selected_features.feature_name = VK_TRUE
#define FEATURE_ENABLE_OR_FAIL(device_builder, feature_name) FEATURE_ENABLE_IF(device_builder, feature_name); else throw Exception {"Device feature '" #feature_name "' not available!"}

class ExtendedFeatureSet {

	public:

		READONLY VkPhysicalDeviceFeatures2KHR vk_features {};

	public:

		/// Set the sType for all chained structs
		ExtendedFeatureSet() {
			vk_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		}

		/// Create the pNext chain and return a pointer to the VkPhysicalDeviceFeatures2KHR
		VkPhysicalDeviceFeatures2KHR* getLinked() {
			return &vk_features;
		}

		/// Get the boring Vulkan 1.0 device features struct
		VkPhysicalDeviceFeatures& getStandard() {
			return vk_features.features;
		}

};
