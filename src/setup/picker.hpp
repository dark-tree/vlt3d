#pragma once

#include "result.hpp"

class Picker {

	protected:

		std::vector<std::string> available;
		std::vector<const char*> selected;

		const std::string type;

		Picker(const std::string& type)
		: type(type) {}

	public:

		Result<std::string> select(const std::string& name) {

			for (const std::string& entry : available) {
				if (entry == name) {
					const char* target = entry.c_str();

					// check if the string is already selected
					for (const char* str : selected) {
						if (strcmp(target, str) == 0) goto after;
					}

					selected.push_back(target);
					after:

					return {type + " '" + name + "' available!", true};
				}
			}

			return {type + " '" + name + "' not available!", false};

		}

		void selectAll() {
			selected.clear();

			for (const std::string& entry : available) {
				selected.push_back(entry.c_str());
			}
		}

		const std::vector<std::string>& getAvailable() const {
			return available;
		}

		const std::vector<const char*>& getSelected() const {
			return selected;
		}

		bool isAnySelected() const {
			return !selected.empty();
		}

	public:

		uint32_t size() {
			return selected.size();
		}

		const char** data() {
			return selected.data();
		}

};

class InstanceExtensionPicker : public Picker {

	public:

		InstanceExtensionPicker() : Picker("Instance extension") {

			// get the number of extension for the given layer (1th nullptr)
			uint32_t count = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

			// check if the extension is present
			if (count != 0) {
				std::vector<VkExtensionProperties> items {count};
				vkEnumerateInstanceExtensionProperties(nullptr, &count, items.data());

				available.reserve(items.size());
				for (auto& item : items) {
					available.emplace_back(item.extensionName);
				}
			}

		}

};

class ValidationLayerPicker : public Picker {

	public:

		ValidationLayerPicker() : Picker("Validation layer") {

			// get the number layers
			uint32_t count = 0;
			vkEnumerateInstanceLayerProperties(&count, nullptr);

			if (count != 0) {
				std::vector<VkLayerProperties> items {count};
				vkEnumerateInstanceLayerProperties(&count, items.data());

				available.reserve(items.size());
				for (auto& item : items) {
					available.emplace_back(item.layerName);
				}
			}

		}

};

class DeviceExtensionPicker : public Picker {

	public:

		DeviceExtensionPicker(const VkPhysicalDevice& device) : Picker("Device extension") {

			// get the number of extension for the given device
			uint32_t count = 0;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

			// check if the extension is present
			if (count != 0) {
				std::vector<VkExtensionProperties> items {count};
				vkEnumerateDeviceExtensionProperties(device, nullptr, &count, items.data());

				available.reserve(items.size());
				for (auto& item : items) {
					available.emplace_back(item.extensionName);
				}
			}

		}

};
