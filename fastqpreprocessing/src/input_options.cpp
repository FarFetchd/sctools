/**
 *  @file   input_options.cpp
 *  @brief  functions for optons and input checking
 *  @author Kishori Konwar
 *  @date   2021-08-11
 ***********************************************/
#include "input_options.h"
#include <experimental/filesystem>

using namespace std;
namespace fs = std::experimental::filesystem;

/** @copydoc read_options_tagsort */
void read_options_tagsort(int argc, char **argv, INPUT_OPTIONS_TAGSORT &options)
{
  int c;
  int i;

  static struct option long_options[] = {
          /* These options set a flag. */
          {"compute-metric",             no_argument,       0, 'm'},
          {"output-sorted-info",         no_argument,       0, 'n'},
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"bam-input",                  required_argument, 0, 'b'},
          {"gtf-file",                   required_argument, 0, 'a'},
          {"temp-folder",                required_argument, 0, 't'},
          {"sorted-output",              required_argument, 0, 'o'},
          {"metric-output",              required_argument, 0, 'M'},
          {"alignments-per-thread",      required_argument, 0, 'p'},
          {"nthreads",                   required_argument, 0, 'T'},
          {"barcode-tag",                required_argument, 0, 'C'},
          {"umi-tag",                    required_argument, 0, 'U'},
          {"gene-tag",                   required_argument, 0, 'G'},
          {"metric-type",                required_argument, 0, 'K'},
          {0, 0, 0, 0}
  };

  // help messages when the user types -h
  const char *help_messages[] = {
           "compute metric, metrics are computed if this option is provided [optional]",
           "sorted output file is produced if this option is provided [optional]",
           "input bam file [required]",
           "gtf file (unzipped) required then metric type is cell [required with metric cell]",
           "temp folder for disk sorting [options: default /tmp]",
           "sorted output file [optional]",
           "metric file, the metrics are output in this file  [optional]",
           "number of alignments per thread [optional: default 1000000], if this number is increased then more RAM is required but reduces the number of file splits",
           "number of threads [optional: default 1]",
           "barcode-tag the call barcode tag [required]", 
           "umi-tag the umi tag [required]: the tsv file output is sorted according the tags in the options barcode-tag, umi-tag or gene-tag",
           "gene-tag the gene tag [required]", 
           "metric type, either \"cell\" or \"gene\" [required]"
  };


  /* getopt_long stores the option index here. */
  int option_index = 0;
  int curr_size = 0;
  while ((c = getopt_long(argc, argv,
                          "b:a:t:no:mM:p:T:C:U:G:K:",
                          long_options,
                          &option_index)) !=- 1
                         )
  {
      // process the option or arguments
      switch (c) {
        case 'm':
            options.compute_metric = 1;
            break;
        case 'n':
            options.output_sorted_info = 1;
            break;
        case 0:
          /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            printf("option %s", long_options[option_index].name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;
        case 'b':
            options.bam_input = string(optarg);
            break;
        case 'a':
            options.gtf_file = string(optarg);
            break;
        case 't':
            options.temp_folder = string(optarg);
            break;
        case 'o':
            options.sorted_output_file = string(optarg);
            break;
        case 'M':
            options.metric_output_file = string(optarg);
            break;
        case 'p':
            options.alignments_per_thread = atoi(optarg);
            break;
        case 'T':
            options.nthreads = atoi(optarg);
            break;
        case 'C':
            options.barcode_tag = string(optarg);
            curr_size = options.tag_order.size();
            options.tag_order[string(optarg)] = curr_size;
            break;
        case 'U':
            options.umi_tag = string(optarg);
            curr_size = options.tag_order.size();
            options.tag_order[string(optarg)] = curr_size;
            break;
        case 'G':
            options.gene_tag = string(optarg);
            curr_size = options.tag_order.size();
            options.tag_order[string(optarg)] = curr_size;
            break;
        case 'K':
            options.metric_type = string(optarg);
            break;
        case '?':
        case 'h':
          i = 0;
          printf("Usage: %s [options] \n", argv[0]);
          while (long_options[i].name != 0) {
            printf("\t--%-20s  %-25s  %-35s\n", long_options[i].name,
                   long_options[i].has_arg == no_argument?
                  "no argument" : "required_argument",
                  help_messages[i]);
            i = i + 1;
          }
          /* getopt_long already printed an error message. */
          exit(0);
          break;
        default:
          abort();
        }
    }

  // Check the options
  // either metric computation or the sorted tsv file must be produced
  if (options.output_sorted_info!=1 && options.compute_metric!=1) {
      error_message("ERROR: The choice of either the  sorted alignment info or metric computation must be specified\n");
      exit(1);
  } else {
      if ( 
           !(options.metric_output_file.size()!=0 && options.compute_metric==1) &&
           !(options.sorted_output_file.size()!=0 && options.output_sorted_info==1)
         ) {
         error_message("ERROR: --compute-metric and --metric-output should be both specified together\n");
         exit(1);
      }
  }

  // metric type must be either of type cell or gene
  if (options.metric_type.size() == 0 || 
      (options.metric_type.compare("cell")!=0 && options.metric_type.compare("gene")!=0)
    ) {
     error_message("ERROR: Metric type must either be \"cell\" or \"gene\"\n");
     exit(1);
  }

  // if metric type is cell then the gtf file must be provided
  if (options.metric_type.compare("cell")==0 && options.gtf_file.size()==0) {
     error_message("ERROR: The gtf file name must be provided with metric_type \"cell\"\n");
     exit(1);
  } 

  // the gtf file should not be gzipped
  std::regex reg1(".gz$", regex_constants::icase);
  if (std::regex_search(options.gtf_file, reg1)) {
     error_message("ERROR: The gtf file must not be gzipped\n");
     exit(1);
  }

  // bam input file must be there
  if (options.bam_input.size() == 0) {
     error_message("ERROR: Must specify a input file name\n");
     exit(1);
  }

  std::stringstream ss;
  // check for input file
  if (not fs::exists(options.bam_input.c_str())) {
     ss.str("");
     ss << "ERROR " << "bam_input" << options.bam_input << " is missing!\n";
     error_message(ss.str().c_str());
     exit(1);
  }

  // check for the temp folder
  if (not fs::exists(options.temp_folder.c_str())) {
     ss.str("");
     ss << "ERROR " << "temp folder " << options.temp_folder <<  " is missing!\n";
     error_message(ss.str().c_str());
     exit(1);
  }

  // check for three distinct tags, barcode, umi and gene_id tags
  if (options.tag_order.size()!=3) {
     error_message("ERROR:  Must have three distinct tags\n");
     exit(1);
  }

  // The size of a set of aligments for in-memory sorting must be positive
  if (options.alignments_per_thread < 1000) {
     error_message("ERROR: The number of alignments per thread must be at least 1000\n");
     exit(1);
  }

 // The number of threads must be between 1 and MAX_THREADS
  if (options.nthreads > MAX_THREADS or options.nthreads < 1) {
     ss << "ERROR: The number of threads must be between 1 and " << MAX_THREADS << "\n";
     error_message(ss.str().c_str());
     exit(1);
  }
}


/** @copydoc read_options_fastqprocess */
void read_options_fastqprocess(int argc, char **argv, INPUT_OPTIONS_FASTQPROCESS &options)
{
  int c;
  int i;

  int verbose_flag = 0;

  static struct option long_options[] = {
          /* These options set a flag. */
          {"verbose",           no_argument,       0, 'v'},
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"barcode-length",    required_argument, 0, 'b'},
          {"umi-length",        required_argument, 0, 'u'},
          {"bam-size",          required_argument, 0, 'B'},
          {"sample-id",         required_argument, 0, 's'},
          {"I1",                required_argument, 0, 'I'},
          {"R1",                required_argument, 0, 'R'},
          {"R2",                required_argument, 0, 'r'},
          {"white-list",        required_argument, 0, 'w'},
          {"output-format",     required_argument, 0, 'F'},
          {0, 0, 0, 0}
  };

  // help messages when the user types -h
  const char *help_messages[] = {
           "verbose messages  ",
           "barcode length [required]",
           "UMI length [required]",
           "output BAM file in GB [optional: default 1 GB]",
           "sample id [required]",
           "I1 [optional]",
           "R1 [required]",
           "R2 [required]",
           "whitelist (from cellranger) of barcodes [required]",
           "output-format : either FASTQ or BAM [required]",
  };


  /* getopt_long stores the option index here. */
  int option_index = 0;
  while ((c = getopt_long(argc, argv,
                          "b:u:B:s:I:R:r:w:F:v",
                          long_options,
                          &option_index)) !=- 1
                         )
  {
      // process the option or arguments
      switch (c) {
        case 'v':
            options.verbose_flag = 1;
            break;
        case 0:
          /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            printf("option %s", long_options[option_index].name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;
        case 'b':
            options.barcode_length = atoi(optarg);
            break;
        case 'u':
            options.umi_length = atoi(optarg);
            break;
        case 'B':
            options.bam_size = atof(optarg);
            break;
        case 's':
            options.sample_id = string(optarg);
            break;
        case 'I':
            options.I1s.push_back(string(optarg));
            break;
        case 'R':
            options.R1s.push_back(string(optarg));
            break;
        case 'r':
            options.R2s.push_back(string(optarg));
            break;
        case 'w':
            options.white_list_file = string(optarg);
            break;
        case 'F':
            options.output_format = string(optarg);
            break;
        case '?':
        case 'h':
          i = 0;
          printf("Usage: %s [options] \n", argv[0]);
          while (long_options[i].name != 0) {
            printf("\t--%-20s  %-25s  %-35s\n", long_options[i].name,
                   long_options[i].has_arg == no_argument?
                  "no argument" : "required_argument",
                  help_messages[i]);
            i = i + 1;
          }
          /* getopt_long already printed an error message. */
          return;
          break;
        default:
          abort();
        }
    }

  // Check the options
  // number of R1 and R2 files should be equal
  bool exit_with_error = false;
  if ((options.R1s.size() != options.R2s.size())) {
     std::cout << "ERROR: Unequal number of R1 and R2 fastq files in input: "
         << "R1 : " << options.R1s.size()
         << "R2 : " << options.R2s.size()
         << std::endl;

     std::cerr << "ERROR: Unequal number of R1 and R2 fastq files in input: "
         << "R1 : " << options.R1s.size()
         << "R2 : " << options.R2s.size()
         << std::endl;

     exit_with_error = true;
  }

  if (options.R1s.size() == 0) {
     std::cout << "ERROR: No R1 file provided\n";
     std::cerr << "ERROR: No R1 file provided\n";

     exit_with_error = true;
  }


  if ((options.I1s.size() != options.R1s.size()) && (options.I1s.size() != 0)) {
     std::cout << "ERROR: Either the number of I1 input files are equal\n"
                  "       to the number of R1 input files, or no I1 input files\n"
                  "       should not be provided at all.\n";
     std::cerr << "ERROR: Either the number of I1 input files are equal\n"
                  "       to the number of R1 input files, or no I1 input files\n"
                  "       should not be provided at all.\n";

     exit_with_error = true;
  }

  // Bam file size must be positive
  if (options.bam_size <= 0) {
     std::cout << "ERROR: Size of a bam file (in GB) cannot be negative\n";
     std::cerr << "ERROR: Size of a bam file (in GB) cannot be negative\n";
     exit_with_error = true;
  }

  // must have a sample id
  if (options.sample_id.size() == 0) {
     std::cout << "ERROR: Must provide a sample id or name\n";
     std::cerr << "ERROR: Must provide a sample id or name\n";
     exit_with_error = true;
  }

  // output options must be FASTQ or BAM
  if (options.output_format!="FASTQ" && options.output_format!="BAM") {
     std::cout << "ERROR: Output-format must be either FASTQ or BAM\n";
     std::cerr << "ERROR: Output-format must be either FASTQ or BAM\n";
     exit_with_error = true;
  }

  // barcode length must be positive
  if (options.barcode_length <= 0) {
     std::cout << "ERROR: Barcode length must be a positive integer\n";
     std::cerr << "ERROR: Barcode length must be a positive integer\n";
     exit_with_error = true;
  }

  // UMI length must be positive
  if (options.umi_length <= 0) {
     std::cout << "ERROR: UMI length must be a positive integer\n";
     std::cerr << "ERROR: UMI length must be a positive integer\n";
     exit_with_error = true;
  }

  // just prints out the files
  if (verbose_flag) {
      if (options.I1s.size()) {
          _print_file_info(options.I1s, std::string("I1"));
      }

      if (options.R1s.size()) {
          _print_file_info(options.R1s, std::string("R1"));
      }

      if (options.R2s.size()) {
          _print_file_info(options.R2s, std::string("R2"));
      }
  }

  if (exit_with_error) {
     exit(1);
  }
}

void read_options_fastq_slideseq(int argc, char **argv, INPUT_OPTIONS_FASTQ_READ_STRUCTURE &options) {
  int c;
  int i;

  int verbose_flag = 0;

  static struct option long_options[] = {
          /* These options set a flag. */
          {"verbose",           no_argument,       0, 'v'},
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"bam-size",          required_argument, 0, 'B'},
          {"read-structure",    required_argument, 0, 'S'},
          {"sample-id",         required_argument, 0, 's'},
          {"I1",                required_argument, 0, 'I'},
          {"R1",                required_argument, 0, 'R'},
          {"R2",                required_argument, 0, 'r'},
          {"white-list",        required_argument, 0, 'w'},
          {"output-format",     required_argument, 0, 'F'},
          {0, 0, 0, 0}
  };

  // help messages when the user types -h
  const char *help_messages[] = {
           "verbose messages  ",
           "output BAM file in GB [optional: default 1 GB]",
           "read structure [required]",
           "sample id [required]",
           "I1 [optional]",
           "R1 [required]",
           "R2 [required]",
           "whitelist (from cellranger) of barcodes [required]",
           "output-format : either FASTQ or BAM [required]",
  };


  /* getopt_long stores the option index here. */
  int option_index = 0;
  while ((c = getopt_long(argc, argv,
                          "B:S:s:I:R:r:w:F:v",
                          long_options,
                          &option_index)) !=- 1
                         )
  {
      // process the option or arguments
      switch (c) {
        case 'v':
            options.verbose_flag = 1;
            break;
        case 0:
          /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            printf("option %s", long_options[option_index].name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;
        case 'B':
            options.bam_size = atof(optarg);
            break;
        case 'S':
            options.read_structure = string(optarg);
            break;
        case 's':
            options.sample_id = string(optarg);
            break;
        case 'I':
            options.I1s.push_back(string(optarg));
            break;
        case 'R':
            options.R1s.push_back(string(optarg));
            break;
        case 'r':
            options.R2s.push_back(string(optarg));
            break;
        case 'w':
            options.white_list_file = string(optarg);
            break;
        case 'F':
            options.output_format = string(optarg);
            break;
        case '?':
        case 'h':
          i = 0;
          printf("Usage: %s [options] \n", argv[0]);
          while (long_options[i].name != 0) {
            printf("\t--%-20s  %-25s  %-35s\n", long_options[i].name,
                   long_options[i].has_arg == no_argument?
                  "no argument" : "required_argument",
                  help_messages[i]);
            i = i + 1;
          }
          /* getopt_long already printed an error message. */
          return;
          break;
        default:
          abort();
        }
    }

  // Check the options
  // number of R1 and R2 files should be equal
  bool exit_with_error = false;
  if ((options.R1s.size() != options.R2s.size()))
  {
     std::cout << "ERROR: Unequal number of R1 and R2 fastq files in input: "
         << "R1 : " << options.R1s.size()
         << "R2 : " << options.R2s.size()
         << std::endl;

     std::cerr << "ERROR: Unequal number of R1 and R2 fastq files in input: "
         << "R1 : " << options.R1s.size()
         << "R2 : " << options.R2s.size()
         << std::endl;

     exit_with_error = true;
  }

  if (options.R1s.size() == 0)
  {
     std::cout << "ERROR: No R1 file provided\n";
     std::cerr << "ERROR: No R1 file provided\n";

     exit_with_error = true;
  }

  if ((options.I1s.size() != options.R1s.size()) && (options.I1s.size() != 0))
  {
     std::cout << "ERROR: Either the number of I1 input files are equal\n"
                  "       to the number of R1 input files, or no I1 input files\n"
                  "       should not be provided at all.\n";
     std::cerr << "ERROR: Either the number of I1 input files are equal\n"
                  "       to the number of R1 input files, or no I1 input files\n"
                  "       should not be provided at all.\n";

     exit_with_error = true;
  }
  // Bam file size must be positive
  if (options.bam_size <= 0)
  {
     std::cout << "ERROR: Size of a bam file (in GB) cannot be negative\n";
     std::cerr << "ERROR: Size of a bam file (in GB) cannot be negative\n";
     exit_with_error = true;
  }
  // must have read structure
  if (options.read_structure.size() == 0)
  {
     std::cout << "ERROR: Must provide read structures\n";
     std::cerr << "ERROR: Must provide read structures\n";
     exit_with_error = true;
  }

  // must have a sample id
  if (options.sample_id.size() == 0) {
     std::cout << "ERROR: Must provide a sample id or name\n";
     std::cerr << "ERROR: Must provide a sample id or name\n";
     exit_with_error = true;
  }

  // output options must be FASTQ or BAM
  if (options.output_format!="FASTQ" && options.output_format!="BAM") {
     std::cout << "ERROR: Output-format must be either FASTQ or BAM\n";
     std::cerr << "ERROR: Output-format must be either FASTQ or BAM\n";
     exit_with_error = true;
  }

  // just prints out the files
  if (verbose_flag) {
      if (options.R1s.size()) {
          _print_file_info(options.R1s, std::string("R1"));
      }

      if (options.R2s.size()) {
          _print_file_info(options.R2s, std::string("R2"));
      }
  }

  if (exit_with_error) {
     exit(1);
  }
}


