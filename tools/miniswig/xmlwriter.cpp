//#include <config.h>

#include <stdexcept>
#include <sstream>
#include "xmlwriter.hpp"

XmlWriter::XmlWriter(std::ostream& outstream) :
    out(outstream), 
    indent(0),
    closetag(),
    lasttag(),
    sections()
{
}

XmlWriter::~XmlWriter()
{
    if(sections.size() > 0) {
        std::cerr << "WARNING: NOT CLOSED: ";
        for(std::vector<std::string>::iterator i = sections.begin();
                i != sections.end(); ++i)
            std::cerr << *i << " ";
        std::cerr << "\n";
    }

    closeTag();
}

void XmlWriter::openTag(const char* name)
{
    newLine();
    out << "<" << name;
    closetag = ">";
    indent++;

    sections.push_back(name);
}

void XmlWriter::closeTag(const char* name)
{
    if(sections.size() == 0)
	throw std::runtime_error("got closeSection without prior openSection.");

    const std::string& lastsection = sections.back();
    if (lastsection != name) {
        std::ostringstream msg;
        msg << "mismatch in open/closeSection. Expected '"
            << lastsection << "' got '" << name << "'";
        throw std::runtime_error(msg.str());
    }
    sections.pop_back();

    indent--;
    newLine();
    // XXX: We should check for consistency here
    out << "</" << name;
    closetag = ">" ;
}

void XmlWriter::writeTag(const char* name)
{
    newLine();
    out << "<" << name;
    closetag = "/>";
    lasttag = name;
}

void XmlWriter::newLine()
{
    if(closetag != "") {
        closeTag();
        for (int i=0;i<indent;i++)
            out << "\t";
    }
}

void XmlWriter::closeTag()
{
    if (closetag != "")
	out << closetag << "\n";
}
