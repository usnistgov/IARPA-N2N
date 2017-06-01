/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) and the Intelligence Advanced Research Projects Activity
 * (IARPA) by employees of the Federal Government in the course of their
 * official duties. Pursuant to title 17 Section 105 of the United States Code,
 * this software is not subject to copyright protection and is in the public
 * domain. NIST and IARPA assume no responsibility whatsoever for its use by
 * other parties, and makes no guarantees, expressed or implied, about its
 * quality, reliability, or any other characteristic.
 */

#include <sys/stat.h>

#include <cmath>

#include <n2nv_identStageOne.h>

#include <be_error.h>
#include <be_io_propertiesfile.h>
#include <be_io_utility.h>
#include <be_memory.h>
#include <be_text.h>

using namespace BiometricEvaluation::Framework::Enumeration;

const std::string N2N::Validation::IdentStageOne::ProcessWorker::LogPathParam{
    "_log"};

static std::string
escapeNewlines(
    const std::string &str)
{
	std::string s{str};
	std::string::size_type pos{std::string::npos};
	while ((pos = s.find("\n")) != std::string::npos)
		s.replace(pos, 1, "\\n");
	return (s);
}

N2N::Validation::IdentStageOne::Arguments
N2N::Validation::IdentStageOne::procargs(
    int argc,
    char *argv[])
{
	static const std::string ConfigDirKey{"Configuration Directory"};
	static const std::string EnrollDirKey{"Enrollment Directory"};
	static const std::string SearchTemplateTypeKey{"Search Template Type"};
	static const std::string OutputDirKey{"Output Directory"};
	static const std::string PrefixKey{"Prefix"};
	static const std::string NumNodesKey{"Number of Nodes"};
	static const std::string NumProcessesKey{"Number of Processes"};
	static const std::string SearchRSPathKey{"Search Template RecordStore"};
	static const std::string StageOneDataRootKey{"Stage One Data Root"};

	static const std::string SearchTemplateTypeValueLatent{"Latent"};
	static const std::string SearchTemplateValueCapture{"Capture"};

	static const std::string NumProcessesDefault{"1"};
	static const std::string PrefixDefault{""};
	static const std::string OutputDirDefault{"."};

	static const std::string usage{"Usage: " + std::string(argv[0]) + " "
	    "<properties.conf>\n\nRequired properties:\n"
	    "\t * " + ConfigDirKey + " = /path/to/directory\n"
	    "\t * " + EnrollDirKey + " = /path/to/directory\n"
	    "\t * " + StageOneDataRootKey + " = /path/to/directory\n"
	    "\t * " + NumNodesKey + " = [1,255]\n"
	    "\t * " + SearchTemplateTypeKey + " = " +
	    SearchTemplateTypeValueLatent + ", " +
	    SearchTemplateValueCapture + "\n"
	    "\t * " + SearchRSPathKey + " = /path/to/RecordStore"
	    "\nOptional properties:\n"
	    "\t * " + NumProcessesKey + " = [1,255] (default: " +
	    NumProcessesDefault + ")\n"
 	    "\t * " + PrefixKey + " = (default: " + PrefixDefault + ")\n"
	    "\t * " + OutputDirKey + " = /path/to/directory (default: " +
	    OutputDirDefault + ")"
	};

	IdentStageOne::Arguments args{};
	if (argc != 2)
		throw BE::Error::StrategyError(usage);

	std::unique_ptr<BE::IO::PropertiesFile> props;
	try {
		props.reset(new BE::IO::PropertiesFile(
		    argv[1], BE::IO::Mode::ReadOnly, {
		    {NumProcessesKey, NumProcessesDefault}}));
	} catch (const BE::Error::Exception &e) {
		throw BE::Error::StrategyError("Could not open \"" +
		    std::string(argv[1]) + "\" (" + e.whatString() + ")");
	}

	/* Configuration directory */
	try {
		args.configDir = props->getProperty(ConfigDirKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    ConfigDirKey + '\n' + usage);
	}

	/* Finalized enrollment set directory */
	try {
		args.enrollDir = props->getProperty(EnrollDirKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    EnrollDirKey + '\n' + usage);
	}
	if (!BE::IO::Utility::pathIsDirectory(args.enrollDir))
		throw BE::Error::ObjectDoesNotExist("Cannot find " +
		    EnrollDirKey + ": " + args.enrollDir + '\n' + usage);

	/* Stage one data directory root */
	try {
		args.stageOneDataRoot = props->getProperty(StageOneDataRootKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    StageOneDataRootKey + '\n' + usage);
	}

	/* Template type */
	try {
		const auto tmplType = props->getProperty(SearchTemplateTypeKey);
		if (BE::Text::caseInsensitiveCompare(tmplType,
		    SearchTemplateTypeValueLatent))
			args.searchTemplateType = InputType::Latent;
		else if (BE::Text::caseInsensitiveCompare(tmplType,
		    SearchTemplateValueCapture))
			args.searchTemplateType = InputType::Capture;
		else
			throw BE::Error::StrategyError("Invalid value for "
			    "property: " + SearchTemplateTypeKey + '\n' +
			    usage);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    ConfigDirKey + '\n' + usage);
	}

	/* Search templates input */
	try {
		args.searchRSPath = props->getProperty(SearchRSPathKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    SearchRSPathKey + '\n' + usage);
	}
	try {
		BE::IO::RecordStore::openRecordStore(args.searchRSPath);
	} catch (const BE::Error::Exception &e) {
		throw BE::Error::StrategyError("Could not open " +
		    SearchRSPathKey + ": " + args.searchRSPath + " (" +
		    e.whatString() + ")\n" + usage);
	}

	/* Number of nodes during finalization */
	try {
		args.numNodes = props->getPropertyAsInteger(NumNodesKey);
		if (args.numNodes == 0)
			throw BE::Error::StrategyError(NumNodesKey + " "
			    "can't be 0");
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    NumNodesKey + '\n' + usage);
	}

	/* Number of nodes during finalization (has default value) */
	args.numProcesses = props->getPropertyAsInteger(NumProcessesKey);
	if (args.numProcesses == 0)
		throw BE::Error::StrategyError(NumProcessesKey + " can't be 0");

	args.prefix = props->getProperty(PrefixKey);

	args.outputDirectory = props->getProperty(OutputDirKey);
	if (BE::IO::Utility::makePath(args.outputDirectory, S_IRWXU | S_IRWXG)
	    != 0)
		throw BE::Error::StrategyError("Could not make directory (" +
		    BE::Error::errorStr() + ')');

	return (args);
}

int
N2N::Validation::IdentStageOne::run(
    const N2N::Validation::IdentStageOne::Arguments &args)
{
	/* Make directory to hold stage one results */
	if (mkdir(args.stageOneDataRoot.c_str(), S_IRWXU | S_IRWXG) != 0)
		throw BE::Error::FileError("Could not create root dir: " +
		    args.stageOneDataRoot + " (" + BE::Error::errorStr() + ')');

	/* Create [1,N] Workers */
	std::vector<std::shared_ptr<BE::Process::WorkerController>> workers;
	BE::Process::ForkManager manager{};
	for (uint8_t i{0}; i < args.numNodes; ++i)
		workers.emplace_back(manager.addWorker(
		    std::make_shared<IdentStageOne::NodeWorker>(i, args)));

	/* fork and wait */
	try {
		manager.startWorkers();
	} catch (const BE::Error::Exception &e) {
		std::cout << "A node encountered an exception (" <<
		    e.whatString() << ")..." << std::endl;
		return (EXIT_FAILURE);
	}

	for (uint8_t i{0}; i < args.numNodes; ++i) {
		if (workers.at(i)->getExitStatus() != EXIT_SUCCESS) {
			std::cout << "Node " << std::to_string(i) << " "
			    "did not exit cleanly." << std::endl;
			return (EXIT_FAILURE);
		}
	}

	/* Make directory to hold directories of merged search data */
	const std::string mergeDir{args.stageOneDataRoot + ".merged"};
	if (mkdir(mergeDir.c_str(), S_IRWXU | S_IRWXG) != 0)
		throw BE::Error::FileError("Could not create merge dir: " +
		    mergeDir +" (" + BE::Error::errorStr() + ')');

	/* Merge results */
	std::shared_ptr<BE::IO::RecordStore> rs;
	try {
		rs = BE::IO::RecordStore::openRecordStore(args.searchRSPath);
	} catch (const BE::Error::Exception &e) {
		/* Remove unused directories */
		try {
			BE::IO::Utility::removeDirectory(args.stageOneDataRoot);
			BE::IO::Utility::removeDirectory(mergeDir);
		} catch (...) {}
		throw;
	}

	BE::IO::RecordStore::Record search;
	while (true) {
		try {
			search = rs->sequence();
		} catch (BE::Error::ObjectDoesNotExist) {
			break;
		}

		/* Make directory to hold merged data */
		const std::string mergeSearchDir{mergeDir + '/' + search.key};
		if (BE::IO::Utility::makePath(mergeSearchDir,
		    S_IRWXU | S_IRWXG) != 0)
			throw BE::Error::FileError("Could not create merged "
			    "search dir: " + mergeSearchDir + " (" +
			    BE::Error::errorStr() + ')');

		for (uint8_t n{0}; n < args.numNodes; ++n) {
			try {
			BE::IO::Utility::copyDirectoryContents(
			    args.stageOneDataRoot + '/' + std::to_string(n) +
			    '/' + search.key, mergeSearchDir, true);
			} catch (BE::Error::Exception &e) {
				std::cout << e.what() << std::endl;
			}
		}
	}

	/* Rename merge directory to value passed for stage one root */
	BE::IO::Utility::removeDirectory(args.stageOneDataRoot);
	if (rename(mergeDir.c_str(), args.stageOneDataRoot.c_str()) != 0)
		throw BE::Error::FileError("Could not rename merge dir (" +
		    mergeDir + " -> " + args.stageOneDataRoot + "): " +
		    BE::Error::errorStr());

	return (EXIT_SUCCESS);
}

/******************************************************************************/

N2N::Validation::IdentStageOne::NodeWorker::NodeWorker(
    uint8_t nodeNumber,
    const IdentStageOne::Arguments &args) :
    _lib{N2N::Interface::getImplementation()},
    _args{args},
    _nodeNumber{nodeNumber}
{
	/* Make directory to hold stage one search results for this node */
	const std::string dataDir{args.stageOneDataRoot + '/' +
	    std::to_string(nodeNumber)};
	if (mkdir(dataDir.c_str(), S_IRWXU | S_IRWXG) != 0)
		throw BE::Error::FileError("Could not create node/process data "
		    "dir: " + dataDir + " (" + BE::Error::errorStr() + ')');
}

int32_t
N2N::Validation::IdentStageOne::NodeWorker::workerMain()
{
	/* Init in node's process before it forks */
	this->_lib->initIdentificationStageOne(this->_args.configDir,
	    this->_args.enrollDir, this->_args.searchTemplateType,
	    this->_nodeNumber);

	/* Create [1,P] Workers */
	BE::Process::ForkManager manager{};
	std::vector<std::shared_ptr<BE::Process::WorkerController>> workers{};
	for (uint8_t i{0}; i < this->_args.numProcesses; ++i) {
		try {
			workers.emplace_back(manager.addWorker(std::make_shared<
			    IdentStageOne::ProcessWorker>(i, this->_nodeNumber,
			    this->_lib, this->_args)));
		} catch (const BE::Error::Exception &e) {
			std::cout << e.whatString() << std::endl;
			return (EXIT_FAILURE);
		}
		workers.back()->setParameter(ProcessWorker::LogPathParam,
		    std::make_shared<std::string>(this->_args.outputDirectory +
		    '/' + this->_args.prefix +
		    std::to_string(this->_nodeNumber) + "-" +
		    std::to_string(i) + ".log" ));
	}

	/* fork and wait */
	try {
		manager.startWorkers();
	} catch (BE::Error::Exception &e) {
		std::cout << "A process from node " <<
		    std::to_string(this->_nodeNumber) << " encountered an "
		    "exception (" << e.whatString() << ")..." << std::endl;
		return (EXIT_FAILURE);
	}

	for (uint8_t i{0}; i < this->_args.numProcesses; ++i) {
		if (workers.at(i)->getExitStatus() != EXIT_SUCCESS) {
			std::cout << "Process " << std::to_string(i) <<
			    " of node " << std::to_string(this->_nodeNumber) <<
			    " did not exit cleanly." << std::endl;
			return (EXIT_FAILURE);
		}
	}

	return (EXIT_SUCCESS);
}

/******************************************************************************/

N2N::Validation::IdentStageOne::ProcessWorker::ProcessWorker(
    uint8_t processNumber,
    uint8_t nodeNumber,
    const std::shared_ptr<N2N::Interface> &lib,
    const IdentStageOne::Arguments &args) :
    _lib{lib},
    _rs{BE::IO::RecordStore::openRecordStore(args.searchRSPath)},
    _maxSearches{static_cast<uint64_t>(std::ceil(
        this->_rs->getCount() / static_cast<float>(args.numProcesses)))},
    _stageOneDataDir{args.stageOneDataRoot + '/' + std::to_string(nodeNumber)}
{
	if (args.numProcesses > this->_rs->getCount())
		throw BE::Error::StrategyError("Not enough processes for data "
		    "(" + std::to_string(args.numProcesses) + " processes, "
		    "and " + std::to_string(this->_rs->getCount()) + " "
		    "searches in " + this->_rs->getPathname() + ')');

	/* Naive partitioning: sequence ahead */
	const uint64_t stopIndex{processNumber * this->_maxSearches};
	try {
		for (uint64_t i{0}; i < stopIndex; ++i)
			static_cast<void>(this->_rs->sequenceKey());
	} catch (BE::Error::ObjectDoesNotExist) {
		/* Too few records for this many processes */
	}
}

int32_t
N2N::Validation::IdentStageOne::ProcessWorker::workerMain()
{
	std::unique_ptr<BE::IO::FileLogsheet> log;
	try {
		log = BE::Memory::make_unique<BE::IO::FileLogsheet>(
		    this->getParameterAsString(LogPathParam),
		    "EntryType EntryNum SearchID Time Size APIState RetCode "
		    "RetInfo");
	} catch (BE::Error::Exception &e) {
		std::cout << "Could not create " +
		    this->getParameterAsString(LogPathParam) + ": " +
		    e.whatString() << std::endl;
		return (EXIT_FAILURE);
	}
	std::string logLine{};
	std::string dataDir{};

	/* Allow 5 minutes maximum per call */
	this->_api.getWatchdog()->setInterval(
	    5 * 60 * BE::Time::MicrosecondsPerSecond);

	for (uint64_t i{0}; i < this->_maxSearches; ++i) {
		/* Get next search template */
		BE::IO::RecordStore::Record record;
		try {
			record = this->_rs->sequence();
		} catch (BE::Error::ObjectDoesNotExist) {
			break;
		}

		/* Make dir to hold results for this node's search results */
		dataDir = this->_stageOneDataDir + '/' + record.key;
		if (mkdir(dataDir.c_str(), S_IRWXU | S_IRWXG) != 0) {
			std::cout << "Could not create dir for search key: " +
			    dataDir + " (" + BE::Error::errorStr() + ')' <<
			    std::endl;
			return (EXIT_FAILURE);
		}

		const auto result = this->_api.call([&]() -> N2N::ReturnStatus {
			return (this->_lib->identifyTemplateStageOne(
			    record.key, record.data, dataDir));
		});

		/* Logging */
		logLine.clear();
		logLine += record.key + ' ' +
		    std::to_string(result.elapsed) + ' ' +
		    std::to_string(BE::IO::Utility::sumDirectoryUsage(
		        dataDir)) + ' ' +
		    std::to_string(to_int_type(result.currentState)) + ' ';
		if (result)
			logLine += std::to_string(static_cast<
			    std::underlying_type<N2N::StatusCode>::type>(
			    result.status.code)) + " [<[" +
			    escapeNewlines(result.status.info) + "]>]";
		else
			logLine += "NA [<[]>]";

		*log << logLine;
		log->newEntry();
	}

	return (EXIT_SUCCESS);
}

/******************************************************************************/

int
main(
    int argc,
    char *argv[])
try {
	using namespace N2N::Validation;
	return (IdentStageOne::run(IdentStageOne::procargs(argc, argv)));
} catch (const BE::Error::Exception &e) {
	std::cout << e.what() << std::endl;
	return (EXIT_FAILURE);
}
