#pragma once

#include "external.hpp"

template<typename T>
class Pyramid {

	private:

		std::vector<std::vector<T>> pyramid;

	public:

		class View {

			private:

				Pyramid& pyramid;
				int index;

				friend class Pyramid;

				View(Pyramid& pyramid)
				: pyramid(pyramid), index(0) {}

			public:

				void up() {
					index ++;
				}

				std::set<T> collect() {
					std::set<T> values;

					for (int i = index; i < pyramid.size(); i ++) {
						for (T& value : pyramid.get(i)) values.insert(value);
					}

					return values;
				}

		};

		void push() {
			pyramid.emplace_back();
		}

		bool empty() {
			return pyramid.empty();
		}

		int size() {
			return pyramid.size();
		}

		std::vector<T>& get(int index) {
			return pyramid.at(index);
		}

		std::vector<T>& top() {
			return pyramid.back();
		}

		void append(const T& value) {
			top().push_back(value);
		}

		View view() {
			return {*this};
		}

};
