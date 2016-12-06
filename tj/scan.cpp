#include "scan.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#define KMTBLSZ (24*12)

class tj {
	public:
	unsigned n;
	unsigned min, max;
	unsigned kmin, kmax;
	unsigned char kmtbl[KMTBLSZ];
	tj() : min(2400), max(0), kmin(1024), kmax(0) {
		std::memset(kmtbl, 0, KMTBLSZ);
	}
};

static std::map<std::string, tj> table;
static std::map<std::string, unsigned> tjroad, tjcity;
static std::set<std::string> filter;
static std::string tjmaxname;
static unsigned tjtime, tjmaxtime, tjmaxlength = 0;
static unsigned tjtbl[KMTBLSZ];

void split(const std::string &s, char delim, std::vector<std::string> &elems)
{
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim))
		elems.push_back(item);
}

std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

int parse(const char *name, const char *road, const char *from, const char *to, unsigned km)
{
	tjtime = (name[0] - '0') * 1000
	       + (name[1] - '0') * 100
	       + (name[2] - '0') * 10
	       + (name[3] - '0');
	unsigned it = (name[0] - '0') * 120
	            + (name[1] - '0') * 12
		    + (name[2] - '0') * 2
		    + (name[3] - '0') / 5;
	std::string xroad(road);
	if (!filter.empty() && filter.find(xroad) == filter.end())
		return 0;
	auto &j = table[xroad];
	j.kmtbl[it] = km;
	j.n++;
	++tjcity[std::string(from)];
	++tjcity[std::string(to)];
	if (tjtime > j.max)
		j.max = tjtime;
	if (tjtime < j.min)
		j.min = tjtime;
	if (km > j.kmax)
		j.kmax = km;
	if (km < j.kmin)
		j.kmin = km;
	tjtbl[it] += km;
	return 0;
}

void tjstat(void)
{
	unsigned tjam = 0;
	for (const auto &x : table) {
		unsigned tmin = x.second.min, tmax = x.second.max;
		printf("%-5s: %2u:%02u %2u:%02u %2u %2u\n", x.first.c_str(), tmin / 100, tmin % 100, tmax / 100, tmax % 100, x.second.kmin, x.second.kmax);
		bool jam = 0;
		unsigned i, j, min, max, avg, n, njam = 0, timemax = 0;
		for (i = j = max = 0, min = 1024, avg = 0, n = 0; i < KMTBLSZ; ++i) {
			unsigned item = x.second.kmtbl[i];
			if (item) {
				if (!jam) {
					tjroad[x.first]++;
					jam = 1;
					j = i;
					++njam;
				}
				if (item > max) {
					max = item;
					timemax = i;
				}
				if (item < min)
					min = item;
				avg += item;
				++n;
			} else if (jam) {
				jam = 0;
				if (max > tjmaxlength) {
					tjmaxlength = max;
					tjmaxname = x.first;
					tjmaxtime = timemax;
				}
				if (flags & OPT_LIST)
					printf("       %2u:%02u %2u:%02u %2u %2u %2u\n", j / 12, 5 * (j % 12), (i - 1) / 12, 5 * ((i - 1) % 12), min, max, avg / n);
				min = 1024;
				avg = n = max = 0;
			}
		}
		if (jam) {
			jam = 0;
			if (max > tjmaxlength) {
				tjmaxlength = max;
				tjmaxname = x.first;
				tjmaxtime = timemax;
			}
			if (flags & OPT_LIST)
				printf("       %2u:%02u %2u:%02u %2u %2u %2u\n", j / 12, 5 * (j % 12), (i - 1) / 12, 5 * ((i - 1) % 12), min, max, avg / n);
			min = 1024;
			avg = n = max = 0;
		}
		if (njam && (flags & OPT_LIST))
			printf("%u jams\n", njam);
		tjam += njam;
	}
	printf("Total jams: %u\n", tjam);
	unsigned max = 0, cmax = 0;
	std::string sroad, scity;
	for (const auto &x : tjroad) {
		unsigned v = x.second;
		if (v > max) {
			max = v;
			sroad = x.first;
		}
	}
	for (const auto &x : tjcity) {
		unsigned v = x.second;
		if (v > cmax) {
			cmax = v;
			scity = x.first;
		}
	}
	printf("Popular highway: %s (%u jams)\n", sroad.c_str(), max);
	printf("Popular city: %s (%u hits)\n", scity.c_str(), cmax);
	printf("Longest jam: %s: %ukm at %2u:%02u\n", tjmaxname.c_str(), tjmaxlength, tjmaxtime / 12, 5 * (tjmaxtime % 12));
	unsigned tjpeak = 0, tjpeaklength = 0;
	for (unsigned i = 0; i < KMTBLSZ; ++i)
		if (tjtbl[i] > tjpeaklength) {
			tjpeaklength = tjtbl[i];
			tjpeak = i;
		}
	printf("Peak: %ukm at %2u:%02u\n", tjpeaklength, tjpeak / 12, 5 * (tjpeak % 12));
}

void tjfilter(const char *arg)
{
	std::vector<std::string> list = split(arg, ',');
	for (auto &x : list)
		filter.insert(x);
}
