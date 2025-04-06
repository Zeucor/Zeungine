#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <iostream>
#include <string>

struct DataEntry
{
	int id;
	std::string name;
	double value;
	DataEntry(int i, std::string n, double v) : id(i), name(std::move(n)), value(v) {}
};
// Define tags for accessing indices
struct by_id
{
};
struct by_name
{
};
// Define the multi-index container
typedef boost::multi_index::multi_index_container<
	DataEntry,
	boost::multi_index::indexed_by<

		boost::multi_index::hashed_unique<boost::multi_index::tag<by_id>,
																			boost::multi_index::member<DataEntry, int, &DataEntry::id>>,

		boost::multi_index::hashed_unique<boost::multi_index::tag<by_name>,
																			boost::multi_index::member<DataEntry, std::string, &DataEntry::name>>

		>>
	DataContainer;

int main()
{
	DataContainer container;

	container.insert({1, "apple", 10.5});
	container.insert({2, "banana", 20.0});
	container.insert({5, "orange", 15.7});

	auto& id_index = container.get<by_id>();
	auto it_id = id_index.find(2);
	if (it_id != id_index.end())
	{
		std::cout << "Found by ID 2: Name=" << it_id->name << ", Value=" << it_id->value << std::endl;
	}

	auto& name_index = container.get<by_name>();
	auto it_name = name_index.find("orange");
	if (it_name != name_index.end())
	{
		std::cout << "Found by Name 'orange': ID=" << it_name->id << ", Value=" << it_name->value << std::endl;
	}

	auto result = container.insert({3, "apple", 30.0});
	if (!result.second)
	{
		std::cout << "Insertion failed: Duplicate key." << std::endl;
	}

	return 0;
}
