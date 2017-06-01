/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <iostream>
#include <string>

#include <n2n.h>

int
main(
    int argc,
    char *argv[])
{
	const auto impl = N2N::Interface::getImplementation();

	uint32_t revision{};
	std::string identifier{}, email{};
	impl->getIDs(identifier, revision, email);

	std::cout << "Identifier: " << identifier << std::endl;
	std::cout << "Revision: " << std::to_string(revision) << std::endl;
	std::cout << "E-mail: " << email << std::endl;

	return (EXIT_SUCCESS);
}
