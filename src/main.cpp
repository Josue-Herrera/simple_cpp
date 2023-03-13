#include <algorithm>
#include <numeric>
#include <memory>
#include <optional>
#include <cmath>
#include <vector>
#include <tuple>
#include <iostream>
#include <string_view>
#include <limits>

#include "task_queue.hpp"
#include "prettyprint.hpp"
#include "associated_collection.hpp"
#include "container_algo.hpp"

using dog_key = std::tuple<int, float>;
struct dog {
	dog_key key;
	bool operator==(dog_key const& k) const { return key == k; }
	bool operator==(dog const& e)     const { return key == e.key; }
	bool operator<(dog_key const& k)  const { return key < k; }
	bool operator<(dog const& e)      const { return key < e.key; }
};

using cat_key = std::tuple<char, int>;
struct cat {
	cat_key key;
	bool operator==(cat_key const& k) const { return key == k; }
	bool operator==(cat const& e)     const { return key == e.key; }
	bool operator<(cat_key const& k)  const { return key < k; }
	bool operator<(cat const& e)      const { return key < e.key; }
};

std::ostream& operator<< (std::ostream& os, dog const& t) {
	os << t.key;
	return os;
}
std::ostream& operator<< (std::ostream& os, cat const& t) {
	os << t.key;
	return os;
}

auto main() -> int
{

	associated_collection<dog, cat_key> dogs;
	associated_collection<cat, dog_key> cats;
	dog_key dk{ 1, 0.f}, dk2 {2, 0.f}, dk3{3, 0.f}, dk4{1,1.f};
	dogs.emplace(dog{dk});
	dogs.emplace(dog{dk2});
	dogs.emplace(dog{dk3});
	dogs.emplace(dog{dk4});

	cat_key ck{'p', 0}, ck2{'a',1}, ck3{'p', 2};
	cats.emplace(cat{ck});
	cats.emplace(cat{ck2});
	cats.emplace(cat{ck3});
	
	emplace_associations(dogs, cats, dk, ck);
	emplace_associations(dogs, cats, dk2, ck2);
	emplace_associations(dogs, cats, dk3, ck3);

	emplace_associations(dogs, cats, dk4, ck);
	emplace_associations(dogs, cats, dk4, ck3);

	std::cout << "dogs:\n\t" << dogs << "\n\t" << dogs.associations() << "\n";
	std::cout << "cats:\n\t" << cats << "\n\t" << cats.associations() << "\n";

	std::cout << "\nvisiting all the dogs that are associated with cat " << ck3 << " : ";
	dogs.visit(ck3, [&](auto& doggo){
		std::cout << "\n\t " << doggo << " : ";
		cats.visit(doggo.key, [](auto& cat){
			std::cout << cat << " ";
		});
	});
	std::cout << "\n";

	std::cout << "\n";

	auto kd = ck;
	auto& k = cats.erase(kd);
	std::cout << "\nerased in cats: " << kd << "\n\tneed to remove in dogs " << k << "\n";
	auto& ks = dogs.erase(k);
	std::cout << "\nerased in dogs: " << k << "\n\tneed to remove in cats " << ks << "\n";
	k.clear();
	k = cats.erase(ks);
	std::cout << "\nerased in cats: " << ks << "\n\tneed to remove in dogs " << k << "\n";
	ks.clear();

	std::cout << "dogs:\n\t" << dogs << "\n\t" << dogs.associations() << "\n";
	std::cout << "cats:\n\t" << cats << "\n\t" << cats.associations() << "\n";
	

	
	return 0;
}
