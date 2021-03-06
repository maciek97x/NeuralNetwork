#include "Tensor.h"

Tensor::Tensor(const std::vector<uint32_t>& shape) {
	_shape = shape;

	_size = 1;
	for (auto s : shape) {
		_size *= s;
	}

	_data.insert(_data.end(), _size, 0.0f);
}

Tensor::Tensor(const Tensor& other) {
	_size = other._size;
	_shape = other._shape;
	_data = other._data;
}

Tensor& Tensor::operator=(const Tensor& other) {
	_size = other._size;
	_shape = other._shape;
	_data = other._data;

	return *this;
}

Tensor::Tensor() {
	_size = 1;
	_shape.push_back(1);
	_data.push_back(0.0f);
}

Tensor::~Tensor() {
}

uint32_t Tensor::getDim() const {
	return _shape.size();
}

uint32_t Tensor::getSize() const {
	return _size;
}

std::vector<uint32_t> Tensor::getShape() const {
	return _shape;
}

std::vector<float> Tensor::getData() const {
	return _data;
}

float Tensor::getValue(const std::vector<uint32_t>& idx) const {
	uint32_t flat_idx = 0;
	uint32_t subidx = 0;
	uint32_t subsize = this->_size;
	uint32_t i = 0;

	if (idx.size() != this->_shape.size()) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	for (i = 0; i < this->_shape.size(); ++i) {
		subsize /= this->_shape[i];
		subidx = idx[i];
		if (subidx >= this->_shape[i]) {
			printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
		}
		flat_idx += subsize * subidx;
	}

	return this->_data[flat_idx];
}

void Tensor::setValue(float value, const std::vector<uint32_t>& idx) {
	if (idx.size() != this->_shape.size()) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	uint32_t flat_idx = 0;
	uint32_t subsize = this->_size;
	for (uint32_t i = 0; i < this->_shape.size(); ++i) {
		subsize /= this->_shape[i];
		flat_idx += subsize * idx[i];
	}

	this->_data[flat_idx] = value;
}

void Tensor::setValues(const std::vector<float>& values) {
	if (this->_data.size() != values.size()) {
		printf("EXCEPTION %d: %d %d\n", __LINE__, this->_data.size(), values.size()); throw std::invalid_argument(""); // exception
	}
	this->_data = values;
}

Tensor Tensor::getSubTensor(const std::vector<uint32_t>& axes) const {
	if (axes.size() != this->_shape.size()) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	for (uint32_t i{ 0 }; i < axes.size(); ++i) {
		if ((WHOLE_AXIS != axes[i]) && (this->_shape[i] <= axes[i])) {
			printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
		}
	}

	std::vector<uint32_t> result_shape;
	std::vector<uint32_t> index;

	for (uint32_t i{ 0 }; i < this->_shape.size(); ++i) {
		if (WHOLE_AXIS == axes[i]) {
			result_shape.push_back(this->_shape[i]);
			index.push_back(0);
		}
		else {
			index.push_back(axes[i]);
		}
	}

	Tensor result = Tensor(result_shape);

	while (true) {
		// set value
		std::vector<uint32_t> sub_index;

		for (uint32_t i{ 0 }; i < index.size(); ++i) {
			if (WHOLE_AXIS == axes[i]) {
				sub_index.push_back(index[i]);
			}
		}

		result.setValue(this->getValue(index), sub_index);

		// increment index
		bool inc_next = false;
		for (int32_t i{ static_cast<int32_t>(index.size() - 1) }; i >= 0; --i) {
			if (WHOLE_AXIS == axes[i]) {
				++index[i];
				if (index[i] >= this->_shape[i]) {
					// overflow
					index[i] = 0;
					inc_next = true;
				}
				else {
					inc_next = false;
					break;
				}
			}
		}

		if (inc_next) {
			break;
		}
	}

	return result;
}

Tensor Tensor::getSubTensor(const std::vector<std::vector<uint32_t> >& ranges) const {
	if (ranges.size() != this->_shape.size()) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	for (uint32_t i{ 0 }; i < ranges.size(); ++i) {
		if ( ranges[i].size() > 2) {
			printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
		}
		if (2 == ranges.size()) {
			if (ranges[i][0] >= ranges[i][1]) {
				printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
			}
			if (ranges[i][1] > this->_shape[i]) {
				printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
			}
		}
		if (1 == ranges.size()) {
			if (ranges[i][0] >= this->_shape[i]) {
				printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
			}
		}
	}

	std::vector<uint32_t> result_shape;
	std::vector<uint32_t> index;

	for (uint32_t i{ 0 }; i < ranges.size(); ++i) {
		switch (ranges[i].size()) {
			case 0:
				result_shape.push_back(this->_shape[i]);
				index.push_back(0);
				break;
			case 1:
				index.push_back(ranges[i][0]);
				break;
			case 2:
				result_shape.push_back(ranges[i][1] - ranges[i][0]);
				index.push_back(ranges[i][0]);
				break;
		}
	}
	
	if (result_shape.size() == 0) {
		Tensor result = Tensor();
		result.setValue(this->getValue(index));

		return result;
	}

	Tensor result = Tensor(result_shape);

	while (true) {
		// set value
		std::vector<uint32_t> sub_index;

		for (uint32_t i{ 0 }; i < index.size(); ++i) {
			if (2 == ranges[i].size()) {
				sub_index.push_back(index[i] - ranges[i][0]);
			}
			else if (0 == ranges[i].size()) {
				sub_index.push_back(index[i]);
			}
		}
		
		result.setValue(this->getValue(index), sub_index);

		// increment index
		bool inc_next = false;
		for (int32_t i{ static_cast<int32_t>(index.size() - 1) }; i >= 0; --i) {
			if (1 != ranges[i].size()) {
				++index[i];
			}
			if (0 == ranges[i].size()) {
				if (index[i] >= this->_shape[i]) {
					// overflow
					index[i] = 0;
					inc_next = true;
				}
				else {
					inc_next = false;
					break;
				}
			}
			else if (2 == ranges[i].size()) {
				if (index[i] >= ranges[i][1]) {
					// overfloww
					index[i] = ranges[i][0];
					inc_next = true;
				}
				else {
					inc_next = false;
					break;
				}
			}
		}

		if (inc_next) {
			break;
		}
	}

	return result;
}

void Tensor::setValuesOfSubTensor(const std::vector<uint32_t>& axes, const Tensor& other) {
	if (axes.size() != this->_shape.size()) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	for (uint32_t i{ 0 }; i < axes.size(); ++i) {
		if ((WHOLE_AXIS != axes[i]) && (this->_shape[i] <= axes[i])) {
			printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
		}
	}

	std::vector<uint32_t> index = axes;

	for (uint32_t i{ 0 }; i < index.size(); ++i) {
		if (WHOLE_AXIS == axes[i]) {
			index[i] = 0;
		}
	}

	while (true) {
		// set value
		std::vector<uint32_t> sub_index;

		for (uint32_t i{ 0 }; i < index.size(); ++i) {
			if (WHOLE_AXIS == axes[i]) {
				sub_index.push_back(index[i]);
			}
		}

		this->setValue(other.getValue(sub_index), index);

		// increment index
		bool inc_next = false;
		for (int32_t i{ static_cast<int32_t>(index.size() - 1) }; i >= 0; --i) {
			if (WHOLE_AXIS == axes[i]) {
				++index[i];
				if (index[i] >= this->_shape[i]) {
					// overflow
					index[i] = 0;
					inc_next = true;
				}
				else {
					inc_next = false;
					break;
				}
			}
		}

		if (inc_next) {
			break;
		}
	}
}

void Tensor::setValuesOfSubTensor(const std::vector<std::vector<uint32_t> >& ranges, const Tensor& other) {
	if (ranges.size() != this->_shape.size()) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	for (uint32_t i{ 0 }; i < ranges.size(); ++i) {
		switch (ranges[i].size()) {
			case 0:
				break;
			case 1:
				if (ranges[i][0] >= this->_shape[i]) {
					printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
				}
				break;
			case 2:
				if (ranges[i][0] >= ranges[i][1]) {
					printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
				}
				if (ranges[i][1] > this->_shape[i]) {
					printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
				}
				break;
			default:
				printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
				break;
		}
	}

	std::vector<uint32_t> index(this->_shape.size());

	for (uint32_t i{ 0 }; i < index.size(); ++i) {
		if (0 == ranges[i].size()) {
			index[i] = 0;
		}
		else {
			index[i] = ranges[i][0];
		}
	}

	while (true) {
		// set value
		std::vector<uint32_t> sub_index;

		for (uint32_t i{ 0 }; i < index.size(); ++i) {
			if (2 == ranges[i].size()) {
				sub_index.push_back(index[i] - ranges[i][0]);
			}
			else if (0 == ranges[i].size()) {
				sub_index.push_back(index[i]);
			}
		}
		
		this->setValue(other.getValue(sub_index), index);

		// increment index
		bool inc_next = false;
		for (int32_t i{ static_cast<int32_t>(index.size() - 1) }; i >= 0; --i) {
			if (0 == ranges[i].size()) {
				++index[i];
				if (index[i] >= this->_shape[i]) {
					// overflow
					index[i] = 0;
					inc_next = true;
				}
				else {
					inc_next = false;
					break;
				}
			}
			else if (2 == ranges[i].size()) {
				++index[i];
				if (index[i] >= ranges[i][1]) {
					// overfloww
					index[i] = ranges[i][0];
					inc_next = true;
				}
				else {
					inc_next = false;
					break;
				}
			}
		}

		if (inc_next) {
			break;
		}
	}
}

Tensor Tensor::addPadding(std::vector<uint32_t> axes, std::vector<Padding> paddings, std::vector<uint32_t> counts) const {
	if (axes.size() != paddings.size() || axes.size() != counts.size()) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	if (axes.size() > this->_shape.size()) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	std::vector<uint32_t> result_shape = this->_shape;

	for (uint32_t i{ 0 }; i < axes.size(); ++i) {
		result_shape[axes[i]] += counts[i] * (!!(paddings[i] & Left) + !!(paddings[i] & Right));
	}

	Tensor result = Tensor(result_shape);

	result *= 0.0f;

	std::vector<std::vector<uint32_t> > ranges(this->_shape.size());
	for (uint32_t i{ 0 }; i < axes.size(); ++i) {
		ranges[axes[i]].push_back(counts[i] * (!!(paddings[i] & Left)));
		ranges[axes[i]].push_back(this->_shape[axes[i]] + counts[i] * !!(paddings[i] & Left));
	}

	result.setValuesOfSubTensor(ranges, *this);

	return result;
}

const Tensor Tensor::operator-() const {
	Tensor result = *this;
	return result * (-1.0f);
}

const Tensor Tensor::operator+(const Tensor& other) const {
	Tensor result = *this;
	
	#ifndef SSE
	result += other;
	#else
	if (this->_size == other._size) {
		SSE_vector_add(this->_size, this->_data.data(), other._data.data(), result._data.data());
	}
	else if (1 == other._size) {
		SSE_tensor_add_scalar(this->_size, this->_data.data(), other._data.data(), result._data.data());
	}
	else if (this->validateShapeReversed(other)) {
		SSE_tensor_add(this->_size, this->_data.data(), other._size, other._data.data(), result._data.data());
	} else {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	#endif

	return result;
}

const Tensor Tensor::operator-(const Tensor& other) const {
	Tensor result = *this;
	result -= other;
	return result;
}

const Tensor Tensor::operator*(const Tensor& other) const {
	Tensor result = *this;
	result *= other;
	return result;
}

const Tensor Tensor::operator/(const Tensor& other) const {
	Tensor result = *this;
	result /= other;
	return result;
}

Tensor& Tensor::operator+=(const Tensor& other) {
	if (((this->_shape.size() < other._shape.size()) ||
		 (!this->validateShapeReversed(other))) &&
		(1 != other._size)) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	#ifndef SSE
	for (uint32_t i{ 0 }; i < this->_size; ++i) {
		this->_data[i] += other._data[i % other._size];
	}
	#else	// SSE
	if (this->_size == other._size) {
		SSE_vector_add(this->_size, this->_data.data(), other._data.data(), this->_data.data());
	}
	else if (1 == other._size) {
		SSE_tensor_add_scalar(this->_size, this->_data.data(), other._data.data(), this->_data.data());
	}
	else if (this->validateShapeReversed(other)) {
		SSE_tensor_add(this->_size, this->_data.data(), other._size, other._data.data(), this->_data.data());
	} else {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	#endif	// SSE

	return *this;
}

Tensor& Tensor::operator-=(const Tensor& other) {
	if (((this->_shape.size() < other._shape.size()) ||
		 (!this->validateShape(other))) &&
		(1 != other._size)) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	#ifndef SSE
	for (uint32_t i{ 0 }; i < this->_size; ++i) {
		this->_data[i] -= other._data[i % other._size];
	}
	#else	// SSE
	if (this->_size == other._size) {
		SSE_vector_sub(this->_size, this->_data.data(), other._data.data(), this->_data.data());
	}
	else if (1 == other._size) {
		SSE_tensor_sub_scalar(this->_size, this->_data.data(), other._data.data(), this->_data.data());
	}
	else if (this->validateShapeReversed(other)) {
		SSE_tensor_sub(this->_size, this->_data.data(), other._size, other._data.data(), this->_data.data());
	} else {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	#endif	// SSE

	return *this;
}

Tensor& Tensor::operator*=(const Tensor& other) {
	if (((this->_shape.size() < other._shape.size()) ||
		 (!this->validateShape(other))) &&
		(1 != other._size)) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	#ifndef SSE
	for (uint32_t i{ 0 }; i < this->_size; ++i) {
		this->_data[i] *= other._data[i % other._size];
	}
	#else	// SSE
	if (this->_size == other._size) {
		SSE_vector_mul(this->_size, this->_data.data(), other._data.data(), this->_data.data());
	}
	else if (1 == other._size) {
		SSE_tensor_mul_scalar(this->_size, this->_data.data(), other._data.data(), this->_data.data());
	}
	else if (this->validateShapeReversed(other)) {
		SSE_tensor_mul(this->_size, this->_data.data(), other._size, other._data.data(), this->_data.data());
	} else {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	#endif	// SSE

	return *this;
}

Tensor& Tensor::operator/=(const Tensor& other) {
	if (((this->_shape.size() < other._shape.size()) ||
		 (!this->validateShape(other))) &&
		(1 != other._size)) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	#ifndef SSE
	for (uint32_t i{ 0 }; i < this->_size; ++i) {
		this->_data[i] /= other._data[i % other._size];
	}
	#else	// SSE
	if (this->_size == other._size) {
		SSE_vector_div(this->_size, this->_data.data(), other._data.data(), this->_data.data());
	}
	else if (1 == other._size) {
		SSE_tensor_div_scalar(this->_size, this->_data.data(), other._data.data(), this->_data.data());
	}
	else if (this->validateShapeReversed(other)) {
		SSE_tensor_div(this->_size, this->_data.data(), other._size, other._data.data(), this->_data.data());
	} else {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	#endif	// SSE

	return *this;
}

const Tensor Tensor::operator>(const Tensor& other) const {
	uint32_t i = 0;

	if ((this->_shape.size() < other._shape.size()) ||
		!this->validateShape(other)) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	Tensor result = *this;

	for (i = 0; i < this->_size; ++i) {
		result._data[i] = this->_data[i] > other._data[i % other._size] ? 1.0f: 0.0f;
	}

	return result;
}

const Tensor Tensor::operator<(const Tensor& other) const {
	uint32_t i = 0;

	if ((this->_shape.size() < other._shape.size()) ||
		!this->validateShape(other)) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	Tensor result = *this;

	for (i = 0; i < this->_size; ++i) {
		result._data[i] = this->_data[i] < other._data[i % other._size] ? 1.0f: 0.0f;
	}

	return result;
}

const Tensor Tensor::operator+(float number) const {
	Tensor result = *this;
	
	#ifndef SSE
	result += number;
	#else	// SSE
	SSE_tensor_add_scalar(this->_size, this->_data.data(), &number, result._data.data());
	#endif	// SSE

	return result;
}

const Tensor operator+(float number, const Tensor& other) {
	return other + number;
}

const Tensor Tensor::operator-(float number) const {
	Tensor result = *this;

	#ifndef SSE
	result -= number;
	#else	// SSE
	SSE_tensor_sub_scalar(this->_size, this->_data.data(), &number, result._data.data());
	#endif	// SSE

	return result;
}

const Tensor operator-(float number, const Tensor& other) {
	Tensor result = other;

	#ifndef SSE
	for (uint32_t i = 0; i < other._size; ++i) {
		result._data[i] = number - result._data[i];
	}
	#else	// SSE
	SSE_scalar_sub_tensor(&number, other._size, other._data.data(), result._data.data());
	#endif	// SSE

	return result;
}

const Tensor Tensor::operator*(float number) const {
	Tensor result = *this;

	#ifndef SSE
	result *= number;
	#else	// SSE
	SSE_tensor_mul_scalar(this->_size, this->_data.data(), &number, result._data.data());
	#endif	// SSE

	return result;
}

const Tensor operator*(float number, const Tensor& other) {
	return other * number;
}

const Tensor Tensor::operator/(float number) const {
	Tensor result = *this;

	#ifndef SSE
	result /= number;
	#else	// SSE
	SSE_tensor_div_scalar(this->_size, this->_data.data(), &number, result._data.data());
	#endif	// SSE

	return result;
}

const Tensor operator/(float number, const Tensor& other) {
	Tensor result = other;

	#ifndef SSE
	for (uint32_t i{ 0} ; i < other._size; ++i) {
		result._data[i] = number / result._data[i];
	}
	#else	// SSE
	SSE_scalar_div_tensor(&number, other._size, other._data.data(), result._data.data());
	#endif	// SSE

	return result;
}

Tensor& Tensor::operator+=(float number) {
	#ifndef SSE
	for (uint32_t i{ 0} ; i < this->_size; ++i) {
		this->_data[i] += number;
	}
	#else	// SSE
	SSE_tensor_add_scalar(this->_size, this->_data.data(), &number, this->_data.data());
	#endif	// SSE

	return *this;
}

Tensor& Tensor::operator-=(float number) {
	#ifndef SSE
	for (uint32_t i{ 0 }; i < this->_size; ++i) {
		this->_data[i] -= number;
	}
	#else	// SSE
	SSE_tensor_sub_scalar(this->_size, this->_data.data(), &number, this->_data.data());
	#endif	// SSE

	return *this;
}

Tensor& Tensor::operator*=(float number) {
	#ifndef SSE
	for (uint32_t i{ 0} ; i < this->_size; ++i) {
		this->_data[i] *= number;
	}
	#else	// SSE
	SSE_tensor_mul_scalar(this->_size, this->_data.data(), &number, this->_data.data());
	#endif	// SSE
	return *this;
}

Tensor& Tensor::operator/=(float number) {
	#ifndef SSE
	for (uint32_t i{ 0} ; i < this->_size; ++i) {
		this->_data[i] /= number;
	}
	#else	// SSE
	SSE_tensor_div_scalar(this->_size, this->_data.data(), &number, this->_data.data());
	#endif	// SSE

	return *this;
}

const Tensor Tensor::operator>(float number) const {
	uint32_t i = 0;

	Tensor result = *this;

	for (i = 0; i < this->_size; ++i) {
		result._data[i] = this->_data[i] > number ? 1.0f : 0.0f;
	}

	return result;
}

const Tensor Tensor::operator<(float number) const {
	uint32_t i = 0;

	Tensor result = *this;

	for (i = 0; i < this->_size; ++i) {
		result._data[i] = this->_data[i] > number ? 1.0f : 0.0f;
	}

	return result;
}

const Tensor Tensor::dotProduct(const Tensor& other) const {
	if (this->_shape.size() == 1 && other._shape.size() == 1) {
		// vector inner product
		if (this->_shape[0] != other._shape[0]) {
			printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
		}
		Tensor result = Tensor();

		#ifndef SSE
		result *= 0.0f;
		
		for (uint32_t i = 0; i < this->_size; ++i) {
			result._data[0] += this->_data[i] * other._data[i];
		}
		#else
		float result_value = 0.0f;

		SSE_vector_inner_product(this->_size, this->_data.data(), other._data.data(), &result_value);

		result._data[0] = result_value;
		#endif

		return result;
	}

	if (this->_shape.size() == 2 && other._shape.size() == 2) {
		// matrix multiplication
		if (this->_shape[1] != other._shape[0]) {
			printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
		}
		std::vector<uint32_t> result_shape = { this->_shape[0], other._shape[1] };

		Tensor result = Tensor(result_shape);

		for (uint32_t i = 0; i < result_shape[0]; ++i) {
			for (uint32_t j = 0; j < result_shape[1]; ++j) {
				for (uint32_t k = 0; k < this->_shape[1]; ++k) {
					result._data[i * result_shape[1] + j] += this->_data[i * this->_shape[1] + k] * other._data[k * other._shape[1] + j];
				}
			}
		}

		return result;
	}

	if (this->_shape.size() == 0) {
		// scalar multiplication
		return other * this->_data[0];
	}

	if (other._shape.size() == 0) {
		// scalar multiplication
		return *this * other._data[0];
	}

	if (other._shape.size() == 1) {
		// sum product over the last axis of this and other (vector)
		if (this->_shape[this->_shape.size() - 1] != other._shape[0]) {
			printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
		}
		std::vector<uint32_t> result_shape = this->_shape;
		result_shape.pop_back();

		Tensor result = Tensor(result_shape);

		uint32_t d_i = this->_size / other._shape[0];

		for (uint32_t i = 0; i < other._shape[0]; ++i) {
			for (uint32_t j = 0; j < d_i; ++j) {
				result._data[j] += this->_data[i * d_i + j] * other._data[i];
			}
		}

		return result;
	}

	else {
		// sum product over the last axis of this and the second-to-last axis of other
		if (this->_shape[this->_shape.size()  - 1] != other._shape[other._shape.size()  - 2]) {
			printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
		}

		Tensor result = Tensor();

		//d_i = this->_size / this->_shape[this->_shape.size()  - 1];
		//d_j = other._size / other._shape[other._shape.size()  - 2];

		// TODO (dot product for higher dimensions currently not essential)

		return result;
	}
}

const Tensor Tensor::dotProductTranspose(const Tensor& other) const {
	if ((this->_shape.size() != 2) || (other._shape.size() != 2)) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	// matrix multiplication
	if (this->_shape[1] != other._shape[1]) {
		printf("EXCEPTION %d %d %d\n", __LINE__, this->_shape[1], other._shape[1]); throw std::invalid_argument(""); // exception
	}
	std::vector<uint32_t> result_shape = { this->_shape[0], other._shape[0] };

	Tensor result = Tensor(result_shape);

	#ifndef SSE
	for (uint32_t i = 0; i < result_shape[0]; ++i) {
		for (uint32_t j = 0; j < result_shape[1]; ++j) {
			for (uint32_t k = 0; k < this->_shape[1]; ++k) {
				result._data[i * result_shape[1] + j] += this->_data[i * this->_shape[1] + k] * other._data[j * other._shape[1] + k];
			}
		}
	}
	#else	// SSE
	SSE_tensor_dot_product_transpose(result_shape[0], result_shape[1], this->_shape[1], this->_data.data(), other._data.data(), result._data.data());
	#endif	// SSE

	return result;
}

const Tensor Tensor::tensorProduct(const Tensor& other) const {
	std::vector<uint32_t> result_shape = this->_shape;
	result_shape.insert(result_shape.end(), other._shape.begin(), other._shape.end());

	Tensor result = Tensor(result_shape);

	for (uint32_t i = 0; i < this->_size; ++i) {
		Tensor subresult = Tensor(other);
		subresult *= this->_data[i];
		std::copy(subresult._data.begin(), subresult._data.end(), result._data.begin() + i * subresult._data.size());
	}

	return result;
}

const Tensor Tensor::applyFunction(float (*function)(float)) const {
	Tensor result = *this;

	for (uint32_t i = 0; i < this->_size; ++i) {
		result._data[i] = function(result._data[i]);
	}

	return result;
}

const Tensor Tensor::flatten(uint32_t from_axis) const {
	if (from_axis >= this->_shape.size() ) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	uint32_t subsize = 1;
	for (uint32_t i = from_axis; i < this->_shape.size() ; ++i) {
		subsize *= this->_shape[i];
	}

	std::vector<uint32_t> result_shape;

	for (uint32_t i = 0; i < from_axis; ++i) {
		result_shape.push_back(this->_shape[i]);
	}
	result_shape.push_back(subsize);

	Tensor result = Tensor(result_shape);

	std::copy(this->_data.begin(), this->_data.end(), result._data.begin());

	return result;
}

const Tensor Tensor::Conv2D(const Tensor& other) const {
	if (this->_shape[2] != other._shape[2]) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	std::vector<uint32_t> result_shape = {this->_shape[0] - (other._shape[0] - 1),
										  this->_shape[1] - (other._shape[1] - 1),
										  other._shape[3]};

	Tensor result = Tensor(result_shape);

	for (uint32_t i{ 0 }; i < result_shape[0]; ++i) {
		for (uint32_t j{ 0 }; j < result_shape[1]; ++j) {
			for (uint32_t k{ 0 }; k < result_shape[2]; ++k) {
				Tensor sub_tensor_this = this->getSubTensor({ { i, i + other._shape[0] },
										  					 { j, j + other._shape[1] },
															 {} });
				Tensor sub_tensor_other = other.getSubTensor({ WHOLE_AXIS, WHOLE_AXIS, WHOLE_AXIS, k});

				result.setValue((sub_tensor_this * sub_tensor_other).sum(), { i, j, k});
			}
		}
	}

	return result;
}

const Tensor Tensor::sum(uint32_t axis) const {
	if (axis >= this->_shape.size() ) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	std::vector<uint32_t> result_shape;

	for (uint32_t i = 0; i < this->_shape.size() ; ++i) {
		if (i != axis) {
			result_shape.push_back(this->_shape[i]);
		}
	}

	Tensor result(result_shape);

	uint32_t d_i{ 1u };
	for (uint32_t i{ axis + 1 }; i < this->_shape.size() ; ++i) {
		d_i *= this->_shape[i];
	}
	#ifndef SSE

	uint32_t d_j = axis == this->_shape.size() - 1 ? this->_shape[this->_shape.size()  - 1] : 1;

	uint32_t d_k = d_i * this->_shape[axis];

	for (uint32_t k{ 0 }; k < this->_size / d_k; ++k) {
		for (uint32_t j{ 0 }; j < d_i; ++j) {
			for (uint32_t i{ 0 }; i < this->_shape[axis] ; ++i) {
				result._data[j + d_i * k] += this->_data[d_i * i + d_j * j + d_k * k];
			}
		}
	}
	#else 	// SSE
	if (axis == this->_shape.size() - 1) {
		SSE_tensor_last_axis_sum(this->_size / (d_i * this->_shape[axis]), this->_shape[axis], this->_data.data(), result._data.data());
	}
	else {
		SSE_tensor_axis_sum(this->_size / (d_i * this->_shape[axis]), d_i, this->_shape[axis], this->_data.data(), result._data.data());
	}
	#endif	// SSE

	return result;
}

float Tensor::sum() const {
	float result{ 0.0f };
	
	#ifndef SSE
	for (uint32_t i{ 0 }; i < this->_size; ++i) {
		result += this->_data[i];
	}
	#else 	// SSE
	SSE_tensor_sum(this->_size, this->_data.data(), &result);
	#endif	// SSE

	return result;
}

float Tensor::max() const {
	float result = _data[0];

	for (auto value : _data) {
		if (value > result) {
			result = value;
		}
	}

	return result;
}

float Tensor::average() const {
	float sum = this->sum();

	return sum/_size;

}

const Tensor Tensor::transpose() const {
	if (this->_shape.size()  != 2) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	std::vector<uint32_t> result_shape = { this->_shape[1], this->_shape[0] };

	Tensor result = Tensor(result_shape);

	for (uint32_t i = 0; i < result_shape[0]; ++i) {
		for (uint32_t j = 0; j < result_shape[1]; ++j) {
			result._data[i * result_shape[1] + j] = this->_data[j * result_shape[0] + i];
		}
	}

	return result;
}

const Tensor Tensor::slice(uint32_t axis, uint32_t start_idx, uint32_t end_idx) const {
	if (start_idx >= end_idx) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	if (axis >= this->_shape.size()) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	if (end_idx > this->_shape[axis]) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	std::vector<uint32_t> result_shape = this->_shape;
	result_shape[axis] = end_idx - start_idx;

	Tensor result = Tensor(result_shape);

	uint32_t subsize_2 = 1;
	for (uint32_t i = axis + 1; i < this->_shape.size() ; ++i) {
		subsize_2 *= this->_shape[i];
	}
	uint32_t subsize_1 = this->_shape[axis] * subsize_2;

	uint32_t j = 0;
	for (uint32_t i = 0; i < this->_size; ++i) {
		if (start_idx * subsize_2 <= i % subsize_1 && i % subsize_1 < end_idx * subsize_2) {
			result._data[j] = this->_data[i];
			++j;
		}
	}
	return result;
}

const Tensor Tensor::shuffle() const {
	uint32_t axis = 0; // currently only for first axis
	uint32_t i = 0;
	uint32_t rand_a;
	uint32_t rand_b;
	Tensor result;
	uint32_t subsize = 0;
	uint32_t shuffle_count = 0;
	float* tmp;

	result = *this;

	subsize = this->_size / this->_shape[axis];
	tmp = (float*)malloc(sizeof(float) * subsize);
	if (!tmp) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}

	srand(time(NULL));

	shuffle_count = 2 * this->_shape[axis] + rand() % this->_shape[axis];

	for (i = 0; i < shuffle_count; ++i) {
		rand_a = rand() % this->_shape[axis];
		rand_b = rand() % this->_shape[axis];
		if (rand_a == rand_b) {
			continue;
		}

		memcpy(tmp, &result._data[subsize * rand_a], sizeof(float) * subsize);
		memcpy(&result._data[subsize * rand_a], &result._data[subsize * rand_b], sizeof(float) * subsize);
		memcpy(&result._data[subsize * rand_b], tmp, sizeof(float) * subsize);
	}

	free(tmp);

	return result;
}

const Tensor Tensor::shuffle(uint32_t *pattern) const {
	uint32_t axis = 0; // currently only for first axis
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t* shuffled;
	Tensor result;
	uint32_t subsize = 0;
	float* tmp;

	result = *this;

	subsize = this->_size / this->_shape[axis];
	tmp = (float*)malloc(sizeof(float) * subsize);
	if (!tmp) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	shuffled = (uint32_t*)malloc(sizeof(uint32_t) * this->_shape[axis]);
	if (!shuffled) {
		printf("EXCEPTION %d\n", __LINE__); throw std::invalid_argument(""); // exception
	}
	memset(shuffled, 0, sizeof(uint32_t) * this->_shape[axis]);

	for (i = 0; i < this->_shape[axis]; ++i) {
		if (!shuffled[i]) {
			shuffled[i] = 1;
			j = pattern[i];
			do {
				shuffled[j] = 1;
				
				memcpy(tmp, &result._data[subsize * j], sizeof(float) * subsize);
				memcpy(&result._data[subsize * j], &result._data[subsize * pattern[j]], sizeof(float) * subsize);
				memcpy(&result._data[subsize * pattern[j]], tmp, sizeof(float) * subsize);
			
				j = pattern[j];
			} while (j != i);
		}
	}

	free(tmp);
	free(shuffled);

	return result;
}

const Tensor Tensor::reshape(std::vector<uint32_t> new_shape) const {
	uint32_t new_size = 1;
	for (auto s : new_shape) {
		new_size *= s;
	}
	if (new_size != this->_size) {
		// exception
	}

	Tensor result{ *this };

	result._shape = new_shape;

	return result;
}

void Tensor::print() const {
	printf("Tensor({ ");
	for (uint32_t i{ 0 }; i < this->_shape.size(); ++i) {
		printf("%d ", this->_shape[i]);
	}
	printf("})\n");
}

bool Tensor::validateShape(const Tensor& other) const {
	uint32_t i = 0;
	uint32_t min_dim = 0;

	if (this->_shape.size() < other._shape.size() ) {
		min_dim = this->_shape.size();
	}
	else {
		min_dim = other._shape.size();
	}
	
	for (i = 0; i < min_dim; ++i) {
		if (this->_shape[i] != other._shape[i]) {
			return false;
		}
	}

	return true;
}

bool Tensor::validateShapeReversed(const Tensor& other) const {
	uint32_t i = 0;
	uint32_t min_dim = 0;

	if (this->_shape.size()  < other._shape.size() ) {
		min_dim = this->_shape.size() ;
	}
	else {
		min_dim = other._shape.size() ;
	}
	
	for (i = 0; i < min_dim; ++i) {
		if (this->_shape[this->_shape.size() - i - 1] != other._shape[other._shape.size() - 1 - i]) {
			return false;
		}
	}

	return true;
}