
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
		const char* name;

	protected:

		uint32_t bytes() const {
			return (uint32_t) output.size() * sizeof(uint32_t);
		}

		const uint32_t* data() const {
			return output.data();
		}

	public:

		CompilerResult(const std::vector<uint32_t>& output, const std::string& error, bool successful, Kind kind, const char* name)
		: output(output), error(error), successful(successful), kind(kind), name(name) {}

		ShaderModule create(Device& device) {
			if (!successful) {
				logger::error("Shader compilation failed: ", error);
				throw Exception {"Unable to create shader module!"};
			}

			return {device, data(), bytes(), kind, name};
		}

};

class Compiler {

	private:

		const shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		class CompilerResultBuilder {

			private:

				Kind kind;
				const char* name;
				std::string errors;
				std::vector<uint32_t> output;

			public:

				CompilerResultBuilder(Kind kind, const char* name)
				: kind(kind), name(name) {}

				template<typename T>
				void copyMessages(const shaderc::CompilationResult<T>& result) {
					errors += result.GetErrorMessage();
				}

				template<typename T>
				void copyOutput(const shaderc::CompilationResult<T>& result) {
					std::copy(result.begin(), result.end(), std::back_inserter(output));
				}

				CompilerResult build(bool successful) {
					return {output, errors, successful, kind, name};
				}

		};

	public:

		Compiler() {
			#if defined(NDEBUG)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
			#else
			options.SetGenerateDebugInfo();
			#endif
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
			CompilerResultBuilder builder {kind, unit.c_str()};

			// preprocessor
			const auto presult = compiler.PreprocessGlsl(source, kind.shaderc, unit.c_str(), options);
			builder.copyMessages(presult);

			if (presult.GetCompilationStatus() != shaderc_compilation_status_success) {
				return builder.build(false);
			}

			// the code after the preprocessor stage
			std::string preprocessed {presult.begin(), presult.end()};

			// compiler
			const auto cresult = compiler.CompileGlslToSpv(preprocessed, kind.shaderc, unit.c_str(), options);
			builder.copyMessages(cresult);

			if (cresult.GetCompilationStatus() != shaderc_compilation_status_success) {
				return builder.build(false);
			}

			builder.copyOutput(cresult);
			return builder.build(true);
		}

};
