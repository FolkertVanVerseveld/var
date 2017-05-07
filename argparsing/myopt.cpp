/*
 * dynamic options parser
 *
 * Just stuff all your arguments you want to parse into an option_parser
 */

#include <iostream>
#include <map>
#include <set>
#include <string>

class option_parser final {
public:
	const std::set<std::string> flags, options;
	const std::map<std::string, std::string> aliases;
	std::map<std::string, std::string> enabled_options;
	std::set<std::string> enabled_flags;
	/* these arguments were not recognized */
	std::set<std::string> dangling_arguments;
	bool bad;
	option_parser(
		const std::set<std::string> &flags,
		const std::set<std::string> &options,
		const std::map<std::string, std::string> &aliases
	) : flags(flags), options(options), aliases(aliases), bad(false) {}
	int parse(int argc, char **argv, int start = 1);
};

int option_parser::parse(int argc, char **argv, int start)
{
	int i;
	for (i = start; i < argc; ++i) {
		/* ignore arguments not starting with `-' */
		if (argv[i][0] != '-') {
			dangling_arguments.emplace(argv[i]);
			continue;
		}
		/* stop parsing if argument equals `--' */
		if (argv[i][1] == '-')
			break;
		std::string arg(&argv[i][1]), arg_expanded;
		auto alias_search = this->aliases.find(arg);
		if (alias_search == this->aliases.end())
			arg_expanded = arg;
		else
			arg_expanded = (*alias_search).second;
		auto arg_search = this->flags.find(arg_expanded);
		if (arg_search != this->flags.end()) {
			enabled_flags.emplace(arg_expanded);
			continue;
		}
		/* not a flag, check if it is an option */
		auto opt_search = this->options.find(arg_expanded);
		if (opt_search == this->options.end()) {
			/* give up */
			std::cerr << "Unknown option: " << arg_expanded << '\n';
			bad = true;
			break;
		}
		if (i + 1 >= argc) {
			std::cerr << "Missing required argument for option: " << arg_expanded << '\n';
			bad = true;
			break;
		}
		enabled_options.emplace(arg_expanded, argv[++i]);
	}
	return i;
}

int main(int argc, char **argv)
{
	std::set<std::string> flags {"help", "version"};
	std::set<std::string> options {"path", "name"};
	std::map<std::string, std::string> aliases {
		{"h", "help"},
		{"p", "path"},
		{"n", "name"},
		{"v", "version"},
	};
	option_parser opt(flags, options, aliases);
	opt.parse(argc, argv);
	if (opt.bad)
		return 1;
	for (auto x : opt.enabled_flags)
		std::cout << x << '\n';
	for (auto x : opt.enabled_options)
		std::cout << x.first << ',' << x.second << '\n';
	return 0;
}
