/**
 * \example ex.12.vector/main.cpp
 * Example illustrating access to elements in a \c dash::Vector by
 * global index.
 */

#include <unistd.h>
#include <iostream>
#include <cstddef>
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

#include <libdash.h>
#include <dash/Algorithm.h>

using std::cout;
using std::endl;

template<class T>
void print_vector(dash::Vector<T>& vec, unsigned int id) {
	vec.commit();
	if (dash::myid() == id) {
		cout << "{ ";
		for (auto el: vec) {
		cout << static_cast<T>(el) << " ";
		}
		cout << "}" << endl;
	}
	vec.barrier();
}

template <size_t size>
struct fixed_string {

	char data[size];

	fixed_string(const char* arg = "") {
		std::strncpy(data, arg, size);
	}

	fixed_string& operator=(const char* arg) {
		std::strncpy(data, arg, size);
	}

};

template <size_t size>
std::ostream& operator<<(std::ostream& lhs, const fixed_string<size>& rhs) {
	lhs << const_cast<const char*>(rhs.data);
	return lhs;
}

auto poly_distribution(double n, double a = 0) {
	// a > 0 < 1;
	return [a, n](double x) -> double {
		return std::pow((x+1)/(n),a) - std::pow(x/(n), a);
	};

}

class Bench {

	using clock = std::chrono::high_resolution_clock;

	clock::time_point begin;

	std::vector<clock::duration> durations;
	std::string msg;

public:
	Bench(std::string msg) :
		msg(msg)
	{};

	~Bench() {
		if(dash::myid() == 0) {
		std::sort(durations.begin(), durations.end());
		double mean = std::chrono::duration_cast<std::chrono::microseconds>(durations.at(durations.size() / 2)).count();
		double min = std::numeric_limits<double>::max(), max = 0, sum = 0;
		for(const auto& dur : durations) {
			const double count = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
			sum += count;
			min = std::min(min, count);
			max = std::max(max, count);
		}
		auto avg = sum / durations.size();

		double stdev = 0;
		for(const auto& dur : durations) {
			double count = std::chrono::duration_cast<std::chrono::microseconds>(dur).count() - avg;
			count *= count;
			stdev += count;
		}
		stdev = std::sqrt(stdev / durations.size());


		std::cout << msg << ": "<< durations.size()
			<< " runs with avg " << avg
			<< " us, mean " << mean
			<< " us,  min " << min
			<< " us, max " << max
			<< " us, stdev " << stdev << " us" << std::endl;;
		}
	}
	void start() {
		begin = clock::now();
	}

	void end() {
		auto end = clock::now();
		durations.push_back(end - begin);
	}
};

int main(int argc, char* argv[])
{
	dash::init(&argc, &argv);

	auto myid = dash::myid();
	auto size = dash::size();
	std::cout << "Initialized context with " << size << " ranks." << std::endl;

	auto& team = dash::Team::All();


	if(myid == 0) {
		std::cout << "dash::vector lpush_back with enough capacity" << std::endl;
	}
	{
		dash::Vector<int> vec(1);
		*(vec.lbegin()) = myid;
		print_vector(vec, size -1);
		vec.reserve(4);

		vec.lpush_back(42);
		print_vector(vec, size -1);

		vec.lpush_back(1337);
		print_vector(vec, size -1);
	}


		if(myid == 0) {
		std::cout << "dash::vector lpush_back with no capacity" << std::endl;
	}
	{
		dash::Vector<int> vec(1);
		*(vec.lbegin()) = myid;
		print_vector(vec, size -1);

		vec.lpush_back(42);
		print_vector(vec, size -1);

		vec.lpush_back(1337);

		print_vector(vec, size -1);
	}

	{
		if(myid == 0) {
			std::cout << "dash::vector push_back with capacity" << std::endl;
		}
		dash::Vector<int> vec;
		vec.reserve(team.size());
		vec.push_back(myid);
		vec.push_back(42);
		vec.push_back(1337);

		print_vector(vec, 0);
	}

	{
		if(myid == 0) std::cout << "dash::vector push_back with no capacity" << std::endl;
		dash::Vector<int> vec;
		vec.push_back(myid);
		vec.push_back(42);
		vec.push_back(1337);
 		print_vector(vec, 0);

		std::cout << "local_size = " << vec.lsize() << std::endl;
		if(myid == 0) std::cout << "dash::vector::balance()" << std::endl;
		vec.balance();
// 		std::cout << "local_size = " << vec.lsize() << std::endl;
		print_vector(vec, 0);
	}

	{
		dash::Vector<char> vec;
		if(myid == 0) {
			vec.push_back('f');
			vec.push_back('b');
		}
			vec.commit();
		if(myid == 0) {
			std::cout << "front: " << static_cast<char>(vec.front()) << std::endl;
			std::cout << "back: " << static_cast<char>(vec.back()) << std::endl;
		}
		team.barrier();
	}

	{
		std::vector<int> queue(5, myid);
		if(myid == 0) std::cout << "dash::vector linsert with no capacity" << std::endl;
		dash::Vector<int> vec;
		vec.linsert(queue.begin(), queue.end());
		print_vector(vec, 0);
	}

	{
		std::vector<int> queue(5, myid);
		if(myid == 0) std::cout << "dash::vector linsert with full capacity" << std::endl;
		dash::Vector<int> vec;
		vec.reserve(10);
		vec.linsert(queue.begin(), queue.end());
		print_vector(vec, 0);
	}

	{
		std::vector<int> queue(5, myid);
		if(myid == 0) std::cout << "dash::vector linsert with half capacity" << std::endl;
		dash::Vector<int> vec;
		vec.reserve(2);
		vec.linsert(queue.begin(), queue.end());
		print_vector(vec, 0);
	}

	{
		std::vector<int> queue(5, myid);
		if(myid == 0) std::cout << "dash::vector insert with no capacity" << std::endl;
		dash::Vector<int> vec;
		vec.insert(queue.begin(), queue.end());
		print_vector(vec, 0);
	}

	{
		std::vector<int> queue(5, myid);
		if(myid == 0) std::cout << "dash::vector insert with full capacity" << std::endl;
		dash::Vector<int> vec;
		vec.reserve(10*size);
		vec.insert(queue.begin(), queue.end());
		print_vector(vec, 0);
	}

	{
		std::vector<int> queue(5, myid);
		if(myid == 0) std::cout << "dash::vector insert with half capacity" << std::endl;
		dash::Vector<int> vec;
		vec.reserve(2);
		vec.insert(queue.begin(), queue.end());
		vec.commit();
		print_vector(vec, 0);
	}


	{
		std::vector<int> queue(5, myid);
		if(myid == 0) std::cout << "dash::vector set values with dash::fill" << std::endl;
		dash::Vector<fixed_string<7>> vec(10, "      ");
		dash::fill(vec.begin(), vec.end(), "filled");
		print_vector(vec, 0);
	}


	constexpr size_t max_elements = 100'000'000;
	constexpr size_t max_runs = 100;



	if(myid == 0) std::cout << "timing" << std::endl;
	{
		for(size_t elements = 1; elements < max_elements; elements *= 10) {
			Bench bench("push_back(cached) " + std::to_string(elements) +  " elements");

			for(int runs = 0; runs < max_runs; runs++) {
				dash::Vector<int> vec;
				bench.start();
				for(int i = 0; i < elements / team.size(); i++) {
					vec.push_back(i, dash::vector_strategy_t::CACHE);
				}
				vec.commit();
				bench.end();
			}
		}
	}

        if(myid == 0) std::cout << "timing" << std::endl;
        {
			for(size_t elements = 1; elements < max_elements; elements *= 10) {
				Bench bench("lpush_back(cached) " + std::to_string(elements) + " elements:");
					for(int runs = 0; runs < max_runs; runs++) {
							dash::Vector<int> vec;
							bench.start();
							for(int i = 0; i < elements / team.size(); i++) {
									vec.lpush_back(i, dash::vector_strategy_t::CACHE);
							}
							vec.commit();
							bench.end();
					}
			}
        }


/*
	if(myid == 0) std::cout << "timing" << std::endl;
	{
		for(size_t elements = 1; elements < 1000000; elements *= 10) {
			const auto total_runs = 5;

			std::chrono::microseconds duration(0);
			for(int runs = 0; runs < total_runs; runs++) {
				dash::Vector<int> vec;
				auto begin = std::chrono::high_resolution_clock::now();
				for(int i = 0; i < elements / team.size(); i++) {
					vec.push_back(i, dash::vector_strategy_t::HYBRID);
				}
				vec.commit();
				auto end = std::chrono::high_resolution_clock::now();
				duration += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
			}
			if(myid == 0) {
				std::cout << "push_back(hybrid) elements: " << elements << "; time " << duration.count()/total_runs << "us, (" << duration2.count()/total_runs << "us )" << std::endl;
			}
		}
	}
*/

	if(myid == 0) std::cout << "timing" << std::endl;
	{
		auto dist = poly_distribution(team.size(), 0.3);

		for(int elements = 1; elements < 1000000; elements *= 10) {
			const auto total_runs = 10;

			Bench bench("balance(vector, uneven) " + std::to_string(elements) +" elements");

			size_t size;
			for(int runs = 0; runs < total_runs; runs++) {

				dash::Vector<int> list;
				auto lelem = elements * dist(team.myid());
				std::vector<int> buff(lelem);
				list.linsert(buff.begin(), buff.end());
				list.commit();
				size = list.size();

				bench.start();

				list.balance();
				bench.end();
			}
		}
	}

/*
	if(myid == 0) std::cout << "timing" << std::endl;
	{
		for(size_t elements = 1000; elements < max_elements; elements *= 10) {
			const auto total_runs = max_runs;

			std::chrono::microseconds duration(0);
			for(int runs = 0; runs < total_runs; runs++) {
				dash::List<int> list(0);
				auto begin = std::chrono::high_resolution_clock::now();
				for(int i = 0; i < elements; i++) {
					if(myid == 0) {
						list.push_back(i);
					}
				}
				list.barrier();
				auto end = std::chrono::high_resolution_clock::now();
				duration += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
			}
			if(myid == 0) {
				std::cout << "push_back(list) elements: " << elements << "; time " << duration.count()/total_runs << " us" << std::endl;
			}
		}
	}
*/


	if(myid == 0) std::cout << "timing" << std::endl;
	{
		for(size_t elements = 1; elements < max_elements; elements *= 10) {
			Bench bench("fill(vector) " + std::to_string(elements) + " elements");
			dash::Vector<int> list(elements/team.size());
			for(int runs = 0; runs < max_runs; runs++) {
				size = list.size();
				bench.start();
				dash::fill(list.begin(), list.end(), 0);

				list.barrier();
				bench.end();
			}
		}
	}


	if(myid == 0) std::cout << "timing" << std::endl;
	{
		for(size_t elements = 1; elements < max_elements; elements *= 10) {
			Bench bench("fill(Array) " + std::to_string(elements) + " elements");

			dash::Array<int> list(elements);
			for(int runs = 0; runs < max_runs; runs++) {
				bench.start();
				dash::fill(list.begin(), list.end(), 0);

				list.barrier();
				bench.end();
			}
		}
	}

	if(myid == 0) std::cout << "timing" << std::endl;
	{
		auto dist = poly_distribution(team.size(), 0.3);

		for(size_t elements = 1; elements < max_elements; elements *= 10) {
			const auto total_runs = max_runs;

			Bench bench("fill(vector, uneven) " + std::to_string(elements) + " elements");

			dash::Vector<int> list;
			auto lelem = elements * dist(team.myid());
			std::vector<int> buff(lelem);
			list.linsert(buff.begin(), buff.end());
			list.commit();

			for(int runs = 0; runs < total_runs; runs++) {
				size = list.size();
				bench.start();
				dash::fill(list.begin(), list.end(), 0);

				list.barrier();
				bench.end();
			}
		}
	}


	if(myid == 0) std::cout << "timing" << std::endl;
	{
		for(size_t elements = 1; elements < max_elements; elements *= 10) {
			Bench bench("std::fill(vector) " + std::to_string(elements) + " elements");

			dash::Vector<int> list(elements/team.size());
			for(int runs = 0; runs < max_runs; runs++) {
				size = list.size();
				bench.start();
				std::fill(list.lbegin(), list.lend(), 0);

				list.barrier();
				bench.end();
			}
		}
	}

	if(myid == 0) std::cout << "timing" << std::endl;
	{
		for(size_t elements = 1; elements < max_elements; elements *= 10) {
			Bench bench("std::fill(Array) " + std::to_string(elements) + " elements");

			dash::Array<int> list(elements);
			for(int runs = 0; runs < max_runs; runs++) {
				bench.start();
				std::fill(list.lbegin(), list.lend(), 0);

				list.barrier();
				bench.end();
			}
		}
	}


	team.barrier();

	dash::finalize();

	return 0;
}
