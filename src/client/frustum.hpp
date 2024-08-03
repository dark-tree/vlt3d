#pragma once

#include "external.hpp"

/**
 * https://gist.github.com/podgorskiy/e698d18879588ada9014768e3e82a644
 */
class Frustum {

	private:

		enum Planes {
			Left = 0,
			Right,
			Bottom,
			Top,
			Near,
			Far,
			Count,
			Combinations = Count * (Count - 1) / 2
		};

		glm::vec4 planes[Count];
		glm::vec3 points[8];

		template<Planes i, Planes j>
		static constexpr size_t select = i * (9 - i) / 2 + j - 1;

		template<Planes a, Planes b, Planes c>
		constexpr inline glm::vec3 intersection(const glm::vec3* crosses) const {
			const float dot = glm::dot(glm::vec3(planes[a]), crosses[select<b, c>]);

			return glm::mat3(crosses[select<b, c>], -crosses[select<a, c>], crosses[select<a, b>])
				* glm::vec3(planes[a].w, planes[b].w, planes[c].w)
				* (-1.0f / dot);
		}

	public:

		Frustum() = default;
		Frustum(const glm::mat4& view_projection);

		// http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
		bool testBox3D(const glm::vec3& minp, const glm::vec3& maxp) const;

};
