#pragma once

class Chain {

	private:

		const int px, py, nx, ny;

		constexpr Chain(int px, int py, int nx, int ny)
		: px(px), py(py), nx(nx), ny(ny) {}

	public:

		constexpr int nextX(int x, int pw, int nw) const {
			return pw * px + nw * nx + x;
		}

		constexpr int nextY(int y, int ph, int nh) const {
			return ph * py + nh * ny + y;
		}

		static Chain AFTER;
		static Chain BEFORE;
		static Chain ABOVE;
		static Chain BELOW; // kinda
		static Chain OVER;
};