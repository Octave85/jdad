#include <iostream>
#include <cstdlib>
#include <cstring>
#include "src/jdad.h"

using namespace std;

class JDAD {
	protected:
		parser_t *parser;
		thing_t * data;
		printer_t *printer;
		std::string infile, outfile;

	public:
		JDAD(std::string in, std::string out, printmode_t mode)
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
			if (out.length() > 0)
			{
				outfile = out;
			}
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
			{
				newostream = fopen(outfile.c_str(), "w");
			}

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
	JDAD unit = JDAD("result.json", "", Pretty);

	unit.printOne(unit.parseOne());
	
	return 0;
}