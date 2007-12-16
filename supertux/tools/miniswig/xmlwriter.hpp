#ifndef __XMLWRITER_H__
#define __XMLWRITER_H__

#include <iostream>
#include <vector>
#include <string>

/** This class is a class which helps printing formated xml output.
 * Example:
 *  This sequence:
 *   xml.openTag("world");
 *   xml.writeAttribute("name", "foo");
 *   xml.writeTag("bar");
 *   xml.writeTag("baz");
 *   xml.writeAttribute("name", "boo");
 *   xml.writeAttribute("style", "old");
 *   xml.write("text");
 *   xml.closeTag("world");
 *  results in this output:
 *   <world name="foo">
 *     <bar/>
 *     <baz name="boo" style="old">text</baz>
 *   </world>
 */
class XmlWriter {
public:
    XmlWriter(std::ostream& out);
    ~XmlWriter();

    /** Start a xml tag which contains subtags */
    void openTag(const char* name);
    /** Closes an xml tag with subtags */
    void closeTag(const char* name);

    void writeTag(const char* name);

    template <class T>
      void comment(const T& outp)
      {   // This routine writes just about anything as an XML comment.
	newLine();
	out << "<!-- " << outp ;
	closetag = " -->";
      }


    template<class T>
    void write(const T& text)
    {
        if (closetag[0]=='>') {
            out << ">";
            closetag = "";
        } else if (closetag[0]=='/') {
      	    out << ">"; // eventually we should place a \n here
	    closetag = "</";
	    closetag += lasttag;
	    closetag += ">";
	}
	out << text;
    }

    template<class T>
    void writeAttribute(const char* name, T value)
    {
	out << " " << name << "=\"" << value << "\"";
    }

private:
    void newLine();
    void closeTag();

    std::ostream& out;
    int indent;
    std::string closetag;
    std::string lasttag;
    std::vector<std::string> sections;
};

#endif
