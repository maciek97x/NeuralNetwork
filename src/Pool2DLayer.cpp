#include "Pool2DLayer.h"

Pool2DLayer::Pool2DLayer(std::vector<uint32_t> input_shape, int32_t pool_size, PoolMode pool_mode) {
	_input_shape = input_shape;
	if (2 == _input_shape.size()) {
		_input_shape.push_back(1);
	}
	else if (3 != _input_shape.size()) {
		// exception
	}

    if ((_input_shape[_input_shape.size() - 2] % pool_size) != 0) {
        // exception
    }
    if ((_input_shape[_input_shape.size() - 3] % pool_size) != 0) {
        // exception
    }

	_output_shape = _input_shape;

    _output_shape[_output_shape.size() - 2] /= pool_size;
    _output_shape[_output_shape.size() - 3] /= pool_size;

    _pool_size = pool_size;
    switch (pool_mode) {
        case PoolMode::Max:
            _pool_function = pool_max;
            _pool_function_d = pool_max_d;
            break;
        case PoolMode::Average:
            _pool_function = pool_average;
            _pool_function_d = pool_average_d;
            break;
    }
}

Pool2DLayer::Pool2DLayer(Layer& prev_layer, int32_t pool_size, PoolMode pool_mode) {
	_input_shape = prev_layer.getOutputShape();
	if (2 == _input_shape.size()) {
		_input_shape.push_back(1);
	}
	else if (3 != _input_shape.size()) {
		// exception
	}

    if ((_input_shape[_input_shape.size() - 2] % pool_size) != 0) {
        // exception
    }
    if ((_input_shape[_input_shape.size() - 3] % pool_size) != 0) {
        // exception
    }

	_output_shape = _input_shape;

    _output_shape[_output_shape.size() - 2] /= pool_size;
    _output_shape[_output_shape.size() - 3] /= pool_size;

	this->setPrevLayer(&prev_layer);
	prev_layer.setNextLayer(this);

    _pool_size = pool_size;
    switch (pool_mode) {
        case PoolMode::Max:
            _pool_function = pool_max;
            _pool_function_d = pool_max_d;
            break;
        case PoolMode::Average:
            _pool_function = pool_average;
            _pool_function_d = pool_average_d;
            break;
    }
}

const Tensor Pool2DLayer::forwardPropagation(const Tensor& x) {
    _cached_input = x;

    std::vector<uint32_t> x_shape = x.getShape();
    std::vector<uint32_t> new_shape = {
        1,
        x_shape[x_shape.size() - 3],
        x_shape[x_shape.size() - 2],
        x_shape[x_shape.size() - 1] };
    
    for (uint32_t i = 0; i < x_shape.size() - 3; ++i) {
        new_shape[0] *= x_shape[i];
    }

    Tensor reshaped_x = x.reshape(new_shape);

    std::vector<uint32_t> reshaped_result_shape = new_shape;
    reshaped_result_shape[1] /= _pool_size;
    reshaped_result_shape[2] /= _pool_size;
    Tensor reshaped_result = Tensor(reshaped_result_shape);

    for (uint32_t i = 0; i < reshaped_result_shape[0]; ++i) {
        for (uint32_t x = 0; x < reshaped_result_shape[1]; ++x) {
            for (uint32_t y = 0; y < reshaped_result_shape[2]; ++y) {
                for (uint32_t c = 0; c < reshaped_result_shape[3]; ++c) {
                    reshaped_result.setValue(
                        _pool_function(reshaped_x.getSubTensor(
                            { { i },
                            { _pool_size*x, _pool_size*(x + 1) },
                            { _pool_size*y, _pool_size*(y + 1) },
                            { c }})).getValue(),
                        { i, x, y, c });
                }
            }
        }
    }
    std::vector<uint32_t> result_shape = x_shape;
    result_shape[result_shape.size() - 3] = x_shape[x_shape.size() - 3]/_pool_size;
    result_shape[result_shape.size() - 2] = x_shape[x_shape.size() - 2]/_pool_size;

    Tensor result = reshaped_result.reshape(result_shape);

    _cached_output = result;
    return result;
}

const Tensor Pool2DLayer::backwardPropagation(const Tensor& dx) {
    std::vector<uint32_t> x_shape = _cached_input.getShape();
    std::vector<uint32_t> new_shape = {
        1,
        x_shape[x_shape.size() - 3],
        x_shape[x_shape.size() - 2],
        x_shape[x_shape.size() - 1] };
    
    for (uint32_t i = 0; i < x_shape.size() - 3; ++i) {
        new_shape[0] *= x_shape[i];
    }

    std::vector<uint32_t> reshaped_result_shape = new_shape;
    reshaped_result_shape[1] /= _pool_size;
    reshaped_result_shape[2] /= _pool_size;
    Tensor reshaped_result = Tensor(new_shape);

    Tensor reshaped_cached_input = _cached_input.reshape(new_shape);
    Tensor reshaped_dx = dx.reshape(reshaped_result_shape);
    
    for (uint32_t i = 0; i < reshaped_result_shape[0]; ++i) {
        for (uint32_t x = 0; x < reshaped_result_shape[1]; ++x) {
            for (uint32_t y = 0; y < reshaped_result_shape[2]; ++y) {
                for (uint32_t c = 0; c < reshaped_result_shape[3]; ++c) {
                    reshaped_result.setValuesOfSubTensor(
                        { { i },
                          { _pool_size*x, _pool_size*(x + 1) },
                          { _pool_size*y, _pool_size*(y + 1) },
                          { c }},
                        _pool_function_d(
                            reshaped_cached_input.getSubTensor(
                                { { i },
                                  { _pool_size*x, _pool_size*(x + 1) },
                                  { _pool_size*y, _pool_size*(y + 1) },
                                  { c }}),
                            reshaped_dx.getSubTensor(
                                { {{ i }},
                                  {{ x }},
                                  {{ y }},
                                  {{ c }} })));
                }
            }
        }
    }
    
    Tensor result = reshaped_result.reshape(x_shape);

    return result;
}

void Pool2DLayer::updateWeights(float learning_step) {
    
}

void Pool2DLayer::initCachedGradient() {

}

void Pool2DLayer::summary() const {
	printf("Pool2D Layer\n");
	printf("  in shape:  (*");
	for (uint32_t i{ 0u }; i < _input_shape.size(); ++i) {
		printf(", %d", _input_shape[i]);
	}
	printf(")  ");
	printf("  out shape: (*");
	for (uint32_t i { 0u }; i < _output_shape.size(); ++i) {
		printf(", %d", _output_shape[i]);
	}
	printf(")  total params: %d\n", getParamsCount());
}

uint32_t Pool2DLayer::getParamsCount() const {
    return 0;
}

const Tensor Pool2DLayer::pool_max(const Tensor& x) {
    Tensor result = Tensor();
    result.setValue(x.max());
    return result;
}

const Tensor Pool2DLayer::pool_max_d(const Tensor& x, const Tensor& dx) {
    float max = x.max();
    Tensor result = x;

    std::vector x_data = x.getData();
    float dx_value = dx.getValue();

    std::transform(x_data.begin(), x_data.end(), x_data.begin(),
        [max, dx_value] (float x) {return x == max ? dx_value : 0.0f;});

    float c = 0.0f;
    for (auto v : x_data) {
        if (v != 0) {
            c += 1;
        }
    }

    if (c > 0) {
        std::transform(x_data.begin(), x_data.end(), x_data.begin(),
            [c] (float x) {return x/c;});
    }

    result.setValues(x_data);

    return result;
}

const Tensor Pool2DLayer::pool_average(const Tensor& x) {
    Tensor result = Tensor();
    result.setValue(x.average());
    return result;
}

const Tensor Pool2DLayer::pool_average_d(const Tensor& x, const Tensor& dx) {
    return (x/x.average())*dx.getValue();
}