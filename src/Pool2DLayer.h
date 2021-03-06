#pragma once

#include <cstdlib>
#include <cstring>

#include "Utils.h"
#include "Layer.h"

enum class PoolMode {
	Max,
    Average
};

class Pool2DLayer : public Layer {
public:
	Pool2DLayer(std::vector<uint32_t> input_shape, int32_t pool_size, PoolMode pool_mode);
	Pool2DLayer(Layer& prev_layer, int32_t pool_size, PoolMode pool_mode);
	
	virtual const Tensor forwardPropagation(const Tensor& x);
	virtual const Tensor backwardPropagation(const Tensor& dx);
	virtual void updateWeights(float learning_step);
	virtual void initCachedGradient();
	virtual void summary() const;
	virtual uint32_t getParamsCount() const;

private:
	uint32_t _pool_size;
	const Tensor (*_pool_function)(const Tensor& x);
	const Tensor (*_pool_function_d)(const Tensor& x, const Tensor& dx);

	static const Tensor pool_max(const Tensor& x);
	static const Tensor pool_max_d(const Tensor& x, const Tensor& dx);
	static const Tensor pool_average(const Tensor& x);
	static const Tensor pool_average_d(const Tensor& x, const Tensor& dx);
};