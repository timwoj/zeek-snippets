#include <string.h>
#include <string>
#include <string_view>
#include <memory>

#ifdef MSVC
#include <filesystem>
#else
#include <libgen.h>
#endif

std::string do_dirname(std::string_view s)
	{
#ifdef MSVC
	return std::filesystem::path(path).parent_path().string();
#else
    std::unique_ptr<char[]> tmp{new char[s.size()+1]};
	strncpy(tmp.get(), s.data(), s.size());
	tmp[s.size()] = '\0';

	char* dn = dirname(tmp.get());
    if ( !dn )
        return "";

    std::string res{dn};

	return res;
#endif
	}

int main(int argc, char** argv)
{
    char a_str[] = "/";
    char b_str[] = "///////";
    char c_str[] = "abcd";
    char d_str[] = "/a/b/c";
    char e_str[] = "";
    char f_str[] = "/a////////b///////c";

    auto a = do_dirname(a_str);
    printf("%s\n", a.c_str());

    auto b = do_dirname(b_str);
    printf("%s\n", b.c_str());

    auto c = do_dirname(c_str);
    printf("%s\n", c.c_str());

    auto d = do_dirname(d_str);
    printf("%s\n", d.c_str());

    auto e = do_dirname(e_str);
    printf("%s\n", e.c_str());

    auto f = do_dirname(f_str);
    printf("%s\n", f.c_str());
}
