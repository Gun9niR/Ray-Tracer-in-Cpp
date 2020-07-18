#pragma once
#include"rtweekend.h"
inline double trilinear_interp(vec3 c[2][2][2], double u, double v, double w)
{
	//Hermitian smoothing to reduce the blocky effect
	auto uu = u * u * (3 - 2 * u);
	auto vv = v * v * (3 - 2 * v);
	auto ww = w * w * (3 - 2 * w);

	auto accum = 0.0;
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			for (int k = 0; k < 2; k++) {
				//weight使得格点上的值会根据当前所求点的坐标而变化
				vec3 weight_v(u - i, v - j, w - k);
				accum += (i * uu + (1 - i) * (1 - uu)) * (j * vv + (1 - j) * (1 - vv)) * (k * ww + (1 - k) * (1 - ww)) * dot(c[i][j][k], weight_v);
			}
	return accum;
}

class perlin {
public:
	perlin() {
		ranvec = new vec3[point_count];

		for (int i = 0; i < point_count; i++) {
			ranvec[i] = unit_vector(vec3::random(-1, 1));
		}
		// perma is a randomized sequence from 0~point_count-1
		perm_x = perlin_generate_perm();
		perm_y = perlin_generate_perm();
		perm_z = perlin_generate_perm();
	}

	~perlin() {
		delete[] ranvec;
		delete[] perm_x;
		delete[] perm_y;
		delete[] perm_z;
	}

	double noise(const point3& p) const {  //构造之后，某一定点的noise是多少是确定的
		//取坐标的小数部分
		auto u = p.x() - floor(p.x());
		auto v = p.y() - floor(p.y());
		auto w = p.z() - floor(p.z());

		//取坐标的整数部分
		int i = floor(p.x());
		int j = floor(p.y());
		int k = floor(p.z());
		vec3 c[2][2][2];

		//保持两个轴不变，另一个轴分别上下取整，然后3个轴坐标异或得到ranvec[]下标，再由下标获得一个随机的vec3放进c[][][]
		for (int di = 0; di < 2; di++)
			for (int dj = 0; dj < 2; dj++)
				for (int dk = 0; dk < 2; dk++)
					c[di][dj][dk] = ranvec[
						perm_x[(i + di) & 255] ^
						perm_y[(j + dj) & 255] ^
						perm_z[(k + dk) & 255]
					];

		//三线性插值，返回灰度值
		return trilinear_interp(c, u, v, w);
	}

	double turb(const point3& p, int depth = 7) const {
		auto accum = 0.0;
		auto temp_p = p;
		auto weight = 1.0;

		//每次点坐标*2，权重/2，accum可能大于1
		for (int i = 0; i < depth; i++) {
			accum += weight * noise(temp_p);
			weight *= 0.5;
			temp_p *= 2;
		}

		return fabs(accum);
	}
private:
	static int* perlin_generate_perm() {   //generate a randomized sequence from 0~point_count-1
		auto p = new int[point_count];
		for (int i = 0; i < perlin::point_count; i++)
			p[i] = i;

		permute(p, point_count);
		
		return p;
	}

	static void permute(int* p, int n) {  //shuffle
		for (int i = n - 1; i > 0; i--) {
			int target = random_int(0, i);
			int tmp = p[i];
			p[i] = p[target];
			p[target] = tmp;
		}
	}

	static const int point_count = 256;
	vec3* ranvec;
	int* perm_x;
	int* perm_y;
	int* perm_z;
};