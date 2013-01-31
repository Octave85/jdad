#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <jdad.h>

using namespace std;

class JDADObject {
protected:
	thing_t *obj;
public:
	JDADObject()
	{
	}

};

class JDADArray {

};

class JDADScalar {

};

class JDADParser {
protected:
	parser_t *parser;
	thing_t * data;
	printer_t *printer;
	std::string infile, outfile;

public:
	JDADParser(std::string in, std::string out="", printmode_t mode=Pretty)
	{

		setInfileName(in);
		setOutfileName(out);

		parser = new_parser((jchar *)infile.c_str());
		FILE *ostream = fopen((jchar *)outfile.c_str(), "w");
		printer = new_printer(ostream, mode);
	}

	void setInfileName(std::string in)
	{
		infile = in;
	}

	void setOutfileName(std::string out)
	{
		if (out.length() > 0) {
			outfile = out;
		}
	}

	bool inEOF(void)
	{
		return (bool)parse_eof(parser);
	}

	void reOpenInput(std::string in)
	{
		setInfileName(in);
		parser = parser_reopen(parser, (jchar *)infile.c_str());
	}

	void reOpenOutput(std::string out)
	{
		setOutfileName(out);

		FILE *newostream;
		if (out.length() == 0)
			newostream = stdout;
		else
			newostream = fopen(outfile.c_str(), "w");

		fclose(printer->ostream);

		if (newostream != NULL)
			printer->ostream = newostream;
	}

	thing_t *parseOne(void)
	{
		parse(parser);
		data = parser->data;

		return data;
	}

	void printOne(thing_t *thing)
	{
		print_thing(printer, thing);
	}

};


int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cout << "No filename supplied" << endl;
		exit(-1);
	}

	int c = 0;
	printmode_t mode;

	c = getopt(argc, argv, "o:c");
	mode = (c=='c') ? Compact : Pretty;

	JDADParser unit = JDADParser(argv[1], (argc >= 2 && optind > 1) ? argv[2] : "", mode);
	while ( ! unit.inEOF()) {
		unit.printOne(unit.parseOne());
		cout << std::endl;
	}
	return 0;
}