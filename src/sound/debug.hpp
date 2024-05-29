#pragma once

#include "external.hpp"

const char* alStringError() {
	int error = alGetError();

	if (error == AL_NO_ERROR) {
		return nullptr;
	}

	if (error == AL_INVALID_NAME) return "AL_INVALID_NAME";
	if (error == AL_INVALID_ENUM) return "AL_INVALID_ENUM";
	if (error == AL_INVALID_VALUE) return "AL_INVALID_VALUE";
	if (error == AL_INVALID_OPERATION) return "AL_INVALID_OPERATION";
	if (error == AL_OUT_OF_MEMORY) return "AL_OUT_OF_MEMORY";

	return "UNDEFINED";
}

void alCheckError(const char* usage) {
	const char* error = alStringError();

	if (error != nullptr) {
		logger::error(usage, ": OpenAL error '", error, "' occured!");
	}
}

