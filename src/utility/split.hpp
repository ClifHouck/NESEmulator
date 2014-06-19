#include <sstream>
#include <string>
#include <vector>

// Cribbed from:
// http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c

std::vector<std::string> &
split(const std::string &s, char delim, std::vector<std::string> &elems);
    
std::vector<std::string> 
split(const std::string &s, char delim);
