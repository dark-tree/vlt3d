
#pragma once

#include "external.hpp"
#include "module.hpp"
#include "kind.hpp"
#include "util/exception.hpp"

class CompilerResult {

	private:

		const std::vector<uint32_t> output;
		const std::string error;
		const bool successful;
		const Kind kind;

	protected:

		uint32_t bytes() const {
			return (uint32_t) output.size() * sizeof(uint32_t);
		}

		const uint32_t* data() const {
			return output.data();
		}

	public:

		CompilerResult(const std::vector<uint32_t>& output, const std::string& error, bool successful, Kind kind)
		: output(output), error(error), successful(successful), kind(kind) {}

		ShaderModule create(Device& device) {
			if (!successful) {
				logger::error("shaderc: Shader compilation failed: ", error);
				throw std::runtime_error {"shaderc: Unable to create shader module!"};
			}

			return {device, data(), bytes(), kind};
		}

		bool isSuccessful() const {
			return successful;
		}

		const std::string& getErrorLog() const {
			return error;
		}

};

class Compiler {

	private:

		const shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		class CompilerResultBuilder {

			private:

				Kind kind;
				std::string errors;
				std::vector<uint32_t> output;

			public:

				CompilerResultBuilder(Kind kind)
				: kind(kind) {}

				template<typename T>
				void copyMessags(const shaderc::CompilationResult<T>& result) {
					errors += result.GetErrorMessage();
				}

				template<typename T>
				void copyOutput(const shaderc::CompilationResult<T>& result) {
					std::copy(result.begin(), result.end(), std::back_inserter(output));
				}

				CompilerResult build(bool successful) {
					return {output, errors, successful, kind};
				}

		};

	public:

		Compiler() {
			setOptimization(shaderc_optimization_level_size);
		}

		// shaderc_optimization_level_zero, shaderc_optimization_level_size, shaderc_optimization_level_performance
		void setOptimization(shaderc_optimization_level value) {
			options.SetOptimizationLevel(value);
		}

		// simmilar to adding -Dkey=value
		void setMacro(const std::string& key, const std::string& value) {
			options.AddMacroDefinition(key, value);
		}

		CompilerResult compileFile(const std::string& identifier, Kind kind) {
			std::ifstream stream {identifier};

			if (stream.fail()) {
				throw Exception {"Failed to open shader file '" + identifier + "'"};
			}

			std::stringstream buffer;
			buffer << stream.rdbuf();

			return compileString(identifier, buffer.str(), kind);
		}

		CompilerResult compileString(const std::string& unit, const std::string& source, Kind kind) {
			CompilerResultBuilder builder {kind};

			// preprocessor
			const auto presult = compiler.PreprocessGlsl(source, kind.shaderc, unit.c_str(), options);
			builder.copyMessags(presult);

			if (presult.GetCompilationStatus() != shaderc_compilation_status_success) {
				return builder.build(false);
			}

			// the code after the preprocessor stage
			std::string preprocessed {presult.begin(), presult.end()};

			// compiler
			const auto cresult = compiler.CompileGlslToSpv(preprocessed, kind.shaderc, unit.c_str(), options);
			builder.copyMessags(cresult);

			if (cresult.GetCompilationStatus() != shaderc_compilation_status_success) {
				return builder.build(false);
			}

			builder.copyOutput(cresult);
			return builder.build(true);
		}

};
