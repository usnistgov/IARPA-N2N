IARPA Nail-to-Nail Fingerprint Challenge NIST Matching API Validation
=====================================================================

The IARPA Nail-to-Nail (N2N) validation package is used to help ensure that
template generation and template matching software submitted for the IARPA N2N
Fingerprint Capture Challenge runs as expected on NIST's hardware and to help
participants eliminate errors in their software before submitting to NIST. This
helps reduce the overall runtime of the evaluation and provide a higher level of
confidence that the results presented by NIST are a true measure of the
submitted software.

*For more general information about participation in N2N, please
visit the [IARPA N2N webpage](
https://www.iarpa.gov/challenges/n2n/n2n.html).*

Package Contents
----------------

 * Interaction Required:
     * [`config/`](
       https://github.com/usnistgov/IARPA-N2N/tree/master/src/validation/config):
       Directory where all required configuration files must be placed. You
       might not have any configuration files.
     * [`lib/`](
       https://github.com/usnistgov/IARPA-N2N/tree/master/src/validation/lib):
       Directory where all required libraries must be placed. There must
       be at least one "core" library, and that library **must** follow the
       N2N library naming convention.
     * [`validate`](
       https://github.com/usnistgov/IARPA-N2N/tree/master/src/validation/validate):
       Script to automate running the validation driver and perform limited
       sanity checks on the output.
 * Supporting Files:
     * [`Makefile`](
       https://github.com/usnistgov/IARPA-N2N/tree/master/src/validation/src/Makefile):
       Builds the validation source code files.
     * [`n2n.h`](
       https://github.com/usnistgov/IARPA-N2N/tree/master/src/include/n2n.h):
       API specification for N2N.
     * [`src/`](
       https://github.com/usnistgov/IARPA-N2N/tree/master/src/validation/src/):
       C++ headers and source codes for the validation drivers.
     * `images/`:
       RecordStores of ANSI/NIST-ITL files containing sample imagery. You must
       sign a usage agreement to obtain this directory from NIST, and then place
       it in the `validation` directory.
     * [`VERSION`](
       (https://github.com/usnistgov/IARPA-N2N/blob/master/src/validation/VERSION):
       Version number of the validation package.

Requirements
------------

The N2N validation package is required to be run on CentOS 7.2.1511. This is not
the latest version of CentOS 7, but it is being required to remain consistent
with other NIST evaluations. An ISO of CentOS 7.2.1511 is available from
[CentOS](http://isoredirect.centos.org/centos/7/isos/x86_64/CentOS-7-x86_64-Everything-1511.iso)
and is additionally
[mirrored at NIST](http://nigos.nist.gov:8080/evaluations/CentOS-7-x86_64-Everything-1511.iso)
(MD5: `dba29c59117400b111633be2bf2aaf0e`). The package section of the kickstart
file used on NIST evaluation nodes [is
provided](http://nigos.nist.gov:8080/evaluations/minexiii/packagelist.txt) (MD5:
`cfb08951f13a8ba2a168b85fc6754503`), though you are not required to use it.
However, the following CentOS packages are required to be installed:

`binutils`, `centos-release`, `coreutils`, `curl`, `findutils`, `gawk`, `gcc`,
`grep`, `iputils`, `make`, `sed`, `which`, `yum`

You will also need
[`libbiomeval`](
https://github.com/usnistgov/BiometricEvaluation/releases/download/v10.0/libbiomeval-10.0-2.x86_64.rpm),
[`libbiomeval-devel`](
https://github.com/usnistgov/BiometricEvaluation/releases/download/v10.0/libbiomeval-devel-10.0-2.noarch.rpm),
and  [`rstool`](
https://github.com/usnistgov/BiometricEvaluation/releases/download/v10.0/rstool-2.1-2.x86_64.rpm) from
[BiometricEvaluation](https://github.com/usnistgov/BiometricEvaluation/)
([release 10](
https://github.com/usnistgov/BiometricEvaluation/releases/tag/v10.0)) in order
to build the validation drivers and link against the N2N
API.

It is **highly** suggested that you ensure your submission will build and run as
expected on environments as close as possible to the NIST evaluation machines,
in order to avoid validation delays and disqualification.

How to Run
----------

 1. Place your N2N software libraries in the `lib` directory.
 2. Place any read-only configuration files in the `config` directory.
 3. Decompress the `images` directory received from IARPA/NIST in the root of
    the validation directory (alongside `lib`, `config`, and `src`).
 4. Execute `./validate`.
     * If you do not support template generation and template matching of latent
      fingerprint images, pass the argument `--without-latents`.
 5. If successful, review, sign *and* encrypt the resulting output archive, and
    e-mail it, along with the encrypting identity's public key, to
    [N2NChallenge@nist.gov](mailto:N2NChallenge@nist.gov). If unsuccessful,
    correct any errors detailed in the script output and try again.

Submission Contents
-------------------

Upon successful completion of the N2N validation package, an archive is
generated that must be signed and encrypted before submitting to NIST.
Participants are strongly encouraged to look at the output of this archive to
ensure that information generated is as expected. Validation log files and
fingerprint templates should not be modified, because their contents must be
generated identically on NIST's systems.

Some runtime information is logged, which is also included in the output.
Absolute file paths are present in these files. Participants should feel free to
replace absolute file paths they deem sensitive with relative file paths, so
long as the meaningful validation information remains present. Files that may
contain such paths are **bolded** below.

 * `<stage>.log.debug`: The output of running `time bin/n2nv_<stage>`. This is
   used to record runtime and any thrown exceptions.
 * `<stage>.log`: The machine-parsable output of the different stages of the N2N
   pipeline, showing the return status and other runtime information. Some
   log files are suffixed with node and process numbers to indicate that the
   work was split.
 * **`<stage>.conf`**: Input configuration files to the validation executables.
 * **`compile.log`**: The output of running `time make -C src`, the MD5 checksum
   of the contents of `lib` and `config`, the output of `ldd` on `n2nv_version`,
   and the current version of the operating system.
 * `version.log`: The output of running `n2nv_version`, containing the result of
   calling your library's implementation of `getIDs()`.
 * `enrollment.rs`, `search_capture.rs`, `search_latent.rs`: RecordStores of
   templates created during the validation.
     * You can extract the contents using `rstool` to take a closer look.
 * `stage_one_capture/`, `stage_one_latent/`: Information written during
    identification stage one.
 * `finalized_enrollment_set`: The finalized enrollment set after calling
   `finalizeEnrollment()`.
 * `config/`: A copy of `config`.
 * `lib/`: A copy of `lib`.

Checks Performed
----------------

The following high-level checks are performed by this package:

 * Necessary validation packages have been installed by `yum`.
 * Validation package is at most recent revision level.
 * Appropriate operating system version is used.
 * Appropriately named N2N software library is present.
 * Configuration files are read-only, if applicable.
 * No previous validation attempts exist in the output directory.
 * Software library links properly against the validation driver.
 * ID information in library is reflected correctly in software library name.
 * Templates can be created.
 * All templates are generated without errors.
 * Identification stage one can be split over multiple nodes with multiple
   processes per node.
 * All matching operations return successfully.

While the validation package tries to eliminate errors from the N2N submission,
there are still several ways in which the package might approve you for
submittal but NIST would later reject you:

 * Software links against dynamic libraries in the dynamic linker path that are
   not present on NIST systems.
 * The most current version of the validation package was not used.
 * Library with this reversion number has already been submitted to NIST.
 * Templates created at NIST do not match templates submitted.
 * Similarity scores and candidate lists created at NIST do not match similarity
   scores and candidates submitted.
 * Submitted software is non-deterministic.
 * Submitted validation package is not signed and encrypted.
 * Submitted validation package is not signed and encrypted using the key whose
   key fingerprint was provided with the N2N application.

Communication
-------------

If you found a bug and can provide steps to reliably reproduce it, or if you
have a feature request, please
[open an issue](https://github.com/usnistgov/IARPA-N2N/issues). Other
questions may be addressed to the [NIST N2N team](mailto:N2NChallenge@nist.gov).
General N2N questions should be addressed to the
[IARPA N2N team](mailto:N2NChallenge@iarpa.gov).

License
-------

The items in this repository are released in the public domain. See the
[LICENSE](https://github.com/usnistgov/IARPA-N2N/blob/master/LICENSE.md)
for details.
