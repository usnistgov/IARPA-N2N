<p align="center"><img src="http://nigos.nist.gov:8080/nist/n2n_logo_white.svg.gz" title="N2N Challenge" alt="N2N Challenge Logo" width=30%"></p>

Nail to Nail Fingerprint Capture Challenge
==========================================

Overview
--------
This repository contains the Application Program Interface (API) for
participant-specific one-to-many template generation and identification
algorithms for Intelligence Advanced Research Projects Activity (IARPA)'s
[2017 Nail to Nail Fingerprint Capture Challenge](
https://www.iarpa.gov/challenges/n2n/n2n.html).
This API is based off the API used for
[Fingerprint Vendor Technology Evaluation (FpVTE) 2012](
https://www.nist.gov/itl/iad/image-group/fpvte-2012),
by the [National Institute of Standards and Technology (NIST)](
https://www.nist.gov/itl/iad/image-group), released in the public domain.

Documentation
-------------
An overview of the software evaluation to be run against this API is available
in the [test plan](
https://github.com/usnistgov/IARPA-N2N/blob/master/doc/testplan/testplan.pdf).

Documentation of the classes and methods that comprise the API are available
within the [API header file](
https://github.com/usnistgov/IARPA-N2N/blob/master/src/include/n2n.h),
[on the web](https://pages.nist.gov/IARPA-N2N), and in a [stand-alone document](
https://github.com/usnistgov/IARPA-N2N/blob/master/doc/n2n_api.pdf).

Implementing the API
--------------------
A pure-virtual (abstract) class called `N2N::Interface` has been created.
Participants must implement all methods of `N2N::Interface` in a subclass, and
submit this implementation as a shared library. The name of the library must
follow the instructions in `N2N::Interface::getIDs()`. A testing application
will link against the submitted library, instantiate an instance of the
implementation by calling `N2N::Interface::getImplementation()`, and perform
various template generation and identification operations.

Implementing this API requires the use of
[`libbiomeval`](https://github.com/usnistgov/BiometricEvaluation) ([release 10](
https://github.com/usnistgov/BiometricEvaluation/releases/tag/v10.0)).

Submitting
----------
Submitting an N2N API implementation involves submitting the output of a
*validation package* to NIST via e-mail.

  1. Download the latest [release](
     https://github.com/usnistgov/IARPA-N2N/releases).
  2. Follow the instructions in [src/validation/README.md](
     https://github.com/usnistgov/IARPA-N2N/blob/master/src/validation/README.md).
  3. Encrypt the validation output and [send to NIST](
     mailto:N2NChallenge@nist.gov).

To complete validation, you must have the required validation imagery. NIST will
e-mail you this imagery once you have signed and returned the *N2N Validation
Data Usage Agreement* to IARPA. This dataset is not yet available to the general
public, and as such, the Data Usage Agreement has only been made available to
registered N2N participants.

Important Dates
---------------
 * ~~15 May 2017: Comments regarding API are due (via [GitHub](
                https://github.com/usnistgov/IARPA-N2N/issues) or [e-mail](
                mailto:N2NChallenge@nist.gov)).~~
 * ~~01 June 2017: NIST will begin receiving validation submissions.~~
 * **01 July 2017**: Submission deadline. Due to the time required to create several
                 million templates that cannot overlap with the data collection,
                 this date **cannot** be extended.

Contact
-------
Additional information about the project in general can be obtained by emailing
N2NChallenge@iarpa.gov. Additional information about the API or associated
software evaluation can be obtained by emailing N2NChallenge@nist.gov.

License
-------
This API is released in the public domain. See the
[LICENSE](https://github.com/usnistgov/IARPA-N2N/blob/master/LICENSE.md) and
[DISCLAIMER](https://github.com/usnistgov/IARPA-N2N/blob/master/DISCLAIMER.md)
for details.
