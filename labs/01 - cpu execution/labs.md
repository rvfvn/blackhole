## 13. Follow the Three Experiments

Work on one numbered section at a time. Read a small example, understand or implement its computation, run its command, check correctness, then explain the timing. The current branch includes completed implementations to inspect and modify. Lab 3 contains a copy of the naive matmul from lab2.c as its comparison baseline, so it can be compiled independently. Keep the function declarations and definitions consistent when editing.

### Experiment 1: Read a Matrix Two Ways

**Edit:** `sum_matrix_by_rows()` and `sum_matrix_by_columns()` in lab1.c. The supplied `main()` below them handles timing, validation, and printing. Only implement the two sum functions.

1. Draw `[[0, 1, 2], [3, 4, 5]]`. Its row-major flat storage is `[0, 1, 2, 3, 4, 5]`.
2. Sum by rows: row loop outside, column loop inside. Read indexes `0, 1, 2, 3, 4, 5`.
3. Sum by columns: swap the loops. Read indexes `0, 3, 1, 4, 2, 5`.
4. Keep the index expression `matrix[row * columns + column]` in both versions. Both sums should be 15.
5. Run `./build.sh 1` to compare larger matrices. Confirm every result says `PASS`.

**Stride** is the distance between reads inside the inner loop: one element across a row, or `columns` elements down a column. Multiply by `sizeof(float)` to get bytes. The CPU keeps data in small, fast memories called **caches**. Data arrives in blocks of neighboring values called **cache lines**. Reading neighbors can reuse more of each line. Small matrices may fit in cache and show little difference.

**Reading the results:** Each table row is one square matrix size. `Rows (ms)` and `Columns (ms)` are the shortest times from five measured runs after a warmup; lower is better. A millisecond is one thousandth of a second. `Columns / rows` divides the column time by the row time: 2.00x means columns took twice as long, 1.00x means equal measured time, and a value below 1.00x means columns were faster.

`PASS` means both methods matched the expected sum on every run. This sum acts as a checksum. Inputs include positive and negative fractions, so a negative total is normal. Very short timings are sensitive to clock resolution and background activity. The experiment measures elapsed time, not cache misses directly; use cache locality to explain a possible cause, not as a measured counter.

**Explain:** Which traversal was faster? How might stride and cache locality explain it? Both functions use the same row-major storage; only traversal changes.

### Experiment 2: Write and Measure Naive Matmul

**Edit:** `matmul_naive()` in lab2.c. The supplied `main()` below it handles timing, validation, and printing. Only implement the matmul function.

First calculate a tiny example on paper:

~~~text
A = [[1, 2], [3, 4]]
B = [[5, 6], [7, 8]]
C = [[19, 22], [43, 50]]
C[0,0] = 1*5 + 2*7 = 19
~~~

For each output `C[i,j]`, accumulate `A[i,k] * B[k,j]` over k. Use three loops in i, j, k order. With flat arrays, the reads are `a[i * n + k]` and `b[k * n + j]`, and the output is `c[i * n + j]`.

Run `./build.sh 2`. The benchmark checks the answer against a double-precision reference, then reports seconds, GFLOP/s, and TFLOP/s. **Explain:** Which input reads neighboring values, and which jumps by a row?

### Understand the Matmul Measurements: Layout and FLOP/s

**Storage order** describes how a matrix is arranged in a flat array. For a matrix with `rows` rows and `columns` columns:

- Row-major storage places each row together: `matrix[row * columns + column]`.
- Column-major storage places each column together: `matrix[column * rows + row]`.

**Traversal order** describes which element the loops visit next. The row and column traversal experiment uses row-major storage in both functions; visiting columns first does not change the storage layout.

For example, the matrix `[[1, 2, 3], [4, 5, 6]]` is stored as `[1, 2, 3, 4, 5, 6]` in row-major order and `[1, 4, 2, 5, 3, 6]` in column-major order. A row-major copy of the transpose has the same flat arrangement as a column-major copy of the original matrix. In experiment 3, A stays row-major while the transposed buffer makes B's original columns contiguous. The result is still **A multiplied by B**.

A **FLOP** is one floating-point arithmetic operation, such as an addition or multiplication. A multiply followed by an add counts as two FLOPs, even if the processor uses one fused multiply-add instruction. FLOPs count work; **FLOP/s** measures how much work completes per second.

- **GFLOP/s** means billions (`10^9`) of floating-point operations per second.
- **TFLOP/s** means trillions (`10^12`) of floating-point operations per second.
- `1 TFLOP/s = 1000 GFLOP/s`. A TFLOP without `/s` is a trillion operations, not a rate.

Each of the `n * n` output elements in square matmul needs `n` multiplications and approximately `n` additions. Use the conventional approximate work count `2 * n * n * n`:

~~~text
GFLOP/s = (2.0 * n * n * n) / elapsed_seconds / 1e9
TFLOP/s = (2.0 * n * n * n) / elapsed_seconds / 1e12
~~~

Starting with `2.0` makes the calculation floating-point and avoids computing the entire operation count with integer multiplication first. Use the shared `print_matmul()` helper declared in lib/bench.h to report both units; `benchmark_time_seconds()` only supplies elapsed seconds and does not know how much arithmetic an experiment performs.

Both matmul implementations perform approximately the same arithmetic. If better memory locality halves execution time, achieved GFLOP/s and TFLOP/s double. For example, approximately two billion operations in 0.1 seconds gives 20 GFLOP/s, or 0.020 TFLOP/s; completing that same work in 0.05 seconds gives 40 GFLOP/s, or 0.040 TFLOP/s. The processor's theoretical peak has not changed. These are measured rates for this workload, and a speedup is not guaranteed for every matrix size or machine.

Compare kernel-only speedup (`naive_time / prepared_kernel_time`) and one-use speedup including preparation (`naive_time / (transpose_time + prepared_kernel_time)`). If you report end-to-end throughput, divide the matmul operation count by the combined transpose and multiplication time and label it explicitly. The transpose moves data; it does not add matmul arithmetic to the numerator.

### Experiment 3: Rearrange B and Compare

**Edit:** `transpose_square_matrix()` and `matmul_with_transposed_b()` in lab3.c. The supplied `main()` below them handles timing, validation, and printing. The naive matmul is supplied as a comparison baseline.

Copy each `b[row * n + column]` into `bt[column * n + row]`. Now the inner multiplication loop reads `a[i * n + k]` and `bt[j * n + k]`: both advance through neighboring values. The answer must still be A multiplied by B.

Run `./build.sh 3`. It runs both matmuls and reports the naive time, prepared kernel time, separate transpose time, and directly measured transpose-plus-kernel time.

**Explain:** Did achieved throughput increase? Does the improvement survive including preparation cost? When might reusing the same prepared B help?

### After Each Experiment

Rebuild and run that experiment and check its results. Save a focused commit and note what you learned. Timing, machine-information, and validation helpers are already supplied in lib/bench.c; understand their purpose before studying their implementation in detail. `benchmark_time_seconds()` returns monotonic seconds, and elapsed time is end minus start. Allocation, initialization, and printing belong outside the timed computation.

## 14. Benchmarking Expectations

A benchmark is more than code that prints a time.

### Verify Correctness First

A faster incorrect implementation is not a successful result.

Use checksums or output comparisons so that:

- row and column traversals agree; and
- both matmul versions produce matching output matrices.

Each experiment validates its measured results outside the timed regions.


### Keep Setup Work Outside the Timed Region

Unless you are intentionally measuring setup cost, do not include these inside a kernel timing:

- memory allocation;
- input initialization;
- result printing;
- correctness checking; or
- unrelated file operations.

Transpose time is important, but report it separately from the transposed matmul kernel.

### Run More Than Once

One run can be affected by other programs, scheduling, temperature, power settings, and cache state. Test each important case more than once and state how you summarized repeated measurements.

Possible summaries include:

- minimum time;
- median time; or
- arithmetic mean.

Whichever method you choose, document it in the pull request.

The implemented driver uses one untimed warmup and the minimum of five samples. Short operations need batching to avoid zero-length clock intervals: each standalone transpose sample contains 16 calls, and its reported time is per call. Other samples contain one call. The one-use transpose-plus-matmul interval is measured directly, rather than adding independently selected minimum times. These are warmed measurements.

### Compare Fairly

When comparing two versions:

- use the same inputs;
- use the same compiler command;
- use the same optimization level;
- test on the same machine;
- perform equivalent correctness checks; and
- keep the amount of mathematical work comparable.

### Interpret Before Comparing Computers

First compare patterns on your own machine. Absolute times from two students may differ because their processors, memory systems, compilers, operating environments, and background workloads differ.

The main result is the relationship between variants, not a contest for the smallest number.

## 15. Generate and Inspect Assembly

To generate optimized assembly for the lab:

~~~sh
./build.sh assembly
~~~

Open `lab1.s`, `lab2.s`, or `lab3.s` and find the function you studied, such as `sum_matrix_by_rows` in lab1.s or `matmul_naive` in lab2.s.

Look for:

- register names;
- load instructions;
- store instructions;
- arithmetic instructions;
- comparisons;
- branches; and
- repeated instruction groups corresponding to loops.

Assembly differs between ARM64 and x86-64 and can differ between compiler versions. You are not expected to memorize exact instructions.

The repository ignores generated .s files. Include your observations in the pull-request description instead of committing generated assembly unless the instructor specifically asks for it.

## 16. Save Your Work with Git

Return to the repository root when using the examples below:

~~~sh
cd ../..
~~~

Check your location and branch:

~~~sh
git status
git branch --show-current
~~~

Inspect changes before staging:

~~~sh
git diff
git diff --check
~~~

The second command helps locate whitespace mistakes.

Stage only intentional work. For example:

~~~sh
git add "labs/01 - cpu execution/lab1.c"
git commit -m "Implement row and column traversal with stride comparison"
~~~

Continue making focused commits as you complete the lab. Other useful commit messages might be:

~~~text
Implement row and column matrix traversal
Implement naive and transposed matmul
Integrate CPU benchmarks in main
~~~

Before submission, inspect the final state:

~~~sh
git status
git log --oneline --max-count=10
~~~

Do not commit generated files such as:

- a.out;
- object files;
- generated assembly; or
- editor-specific temporary files.

The repository's .gitignore already excludes the common forms of these artifacts, but git status is still your final check.

## 17. Push Your Branch

Push the personal lab branch to your fork:

~~~sh
git push -u origin lab01-YOUR-USERNAME
~~~

Replace YOUR-USERNAME with the same username used in your branch name.

The -u option connects the local branch to the branch on your fork. Later pushes from the same branch can usually use:

~~~sh
git push
~~~

## 18. Open the Individual Pull Request

After pushing:

1. Open your fork on GitHub.
2. Select **Compare & pull request** if the banner appears.
3. If needed, open the original [rvfvn/blackhole](https://github.com/rvfvn/blackhole) repository and choose to create a pull request across forks.
4. Confirm the destination:
   - base repository: **rvfvn/blackhole**
   - base branch: **main**
5. Confirm the source:
   - head repository: **YOUR-USERNAME/blackhole**
   - compare branch: **lab01-YOUR-USERNAME**
6. Use the title:

~~~text
Lab 01: CPU execution - YOUR NAME
~~~

7. Leave **Allow edits by maintainers** enabled when GitHub offers that option.
8. Complete the pull-request description using the required report below.
9. Select **Create pull request**.

GitHub's official instructions are in [Creating a pull request](https://docs.github.com/en/pull-requests/how-tos/create-pull-requests/creating-a-pull-request).

Submit one pull request per student. Do not combine several students' implementations into one submission.

## 19. Required Pull-Request Report

Your pull-request description is your lab write-up. Do not create a separate shared results file unless the instructor later requests one.

Paste relevant terminal output as text inside fenced code blocks. Include the commands that produced the output. A successful compiler command may print nothing; in that case, state clearly that it completed without warnings or errors.

Use this structure:

~~~~markdown
## Student

Name:
GitHub username:
Computer and operating environment:

## Implementation Summary

Briefly explain what you implemented across the lab.

## Files Changed

List the source files you changed and the purpose of each change.

## Build Command and Compiler Output

Paste the build.sh command you ran. The compiler flags are recorded in build.sh.

~~~text
$ ./build.sh all
PASTE COMPILER OUTPUT HERE
~~~

State whether the build completed with warnings or errors.

## Machine Information

Paste the output produced by machine_info().

~~~text
PASTE MACHINE INFORMATION HERE
~~~

## Benchmark Configuration

Document:

- array sizes;
- matrix sizes;
- inner-loop strides for row and column traversal;
- repetition count;
- compiler and optimization level; and
- how repeated measurements were summarized.

## Terminal Output and Results

Paste the important output from ./build.sh all.

~~~text
$ ./build.sh all
PASTE BENCHMARK OUTPUT HERE
~~~

Summarize the measurements in tables when useful.

### Memory Access: Row and Column Traversal

| Matrix size | Traversal | Inner-loop stride (elements) | Time | Checksum or status |
| --- | --- | ---: | ---: | --- |
| | Rows | 1 | | |
| | Columns | Number of columns | | |

### Matrix Multiplication

| Matrix size | Version | Time | Approximate GFLOP/s | Approximate TFLOP/s | Validation |
| --- | --- | ---: | ---: | ---: | --- |
| | | | | | |

Report transpose time separately.

## Analysis

Answer these questions in your own words:

1. How did row traversal compare with column traversal? Explain the inner-loop strides and how cache locality could affect the results.
2. Did transposing B change matmul performance? Explain why the same operation count can produce different GFLOP/s and TFLOP/s, and distinguish storage order from traversal order.
3. When the transpose cost is included, how does your interpretation change?
4. Which results were surprising or inconsistent?
5. How do these CPU observations motivate specialized accelerators such as Blackhole?

## Assembly Notes

State which source file you compiled to assembly. Identify examples of registers, loads, stores, arithmetic, branches, or loop structure. Exact instruction names may differ by machine.

## Problems, Decisions, and Lab Notes

Describe:

- bugs or compiler warnings you encountered;
- how you corrected them;
- implementation choices you made;
- remaining limitations;
- questions you still have; and
- anything the instructor should consider when integrating the final benchmark.

## Checklist

- [ ] I implemented every experiment.
- [ ] I completed lab1.c, lab2.c, and lab3.c.
- [ ] I built with -O2 and warnings enabled.
- [ ] The final build has no compiler errors.
- [ ] I investigated or resolved compiler warnings.
- [ ] I checked correctness before discussing performance.
- [ ] I included machine information.
- [ ] I included terminal output and benchmark results.
- [ ] I included my analysis and notes.
- [ ] I did not commit generated executables or assembly.
~~~~

Replace every placeholder. Do not submit an empty table or leave PASTE OUTPUT HERE in the final report.

## 20. Respond to Review

The instructor will review your code and report. A review may request corrections, additional measurements, or a clearer explanation.

To respond:

1. Return to your existing lab branch.
2. Make the requested changes.
3. Rebuild and rerun the relevant experiments.
4. Commit the correction.
5. Push the same branch.

For example:

~~~sh
git switch lab01-YOUR-USERNAME
git add "labs/01 - cpu execution/lab1.c" "labs/01 - cpu execution/lab2.c" "labs/01 - cpu execution/lab3.c"
git commit -m "Address Lab 01 review feedback"
git push
~~~

The existing pull request updates automatically when you push to its branch. Do not open a second pull request for review corrections.

The instructor may integrate, adapt, or compare parts of several submissions rather than merging one student's complete implementation unchanged. Your clear function boundaries, benchmark evidence, and written reasoning make that synthesis possible.

## 21. Troubleshooting

### cc: command not found

Return to the platform setup:

- macOS: install the Xcode Command Line Tools.
- Ubuntu, Linux, or WSL: install build-essential.

### lib/bench.h Cannot Be Found

Confirm that:

- you are inside the lab directory;
- lib/bench.h and lib/bench.c exist in the lib directory; and
- the command contains -I..

Use:

~~~sh
pwd
ls
ls lab1.c lab2.c lab3.c lib/bench.h lib/bench.c
~~~

### A Lab File Cannot Be Found

You are probably in the wrong folder. Enter:

~~~sh
cd "labs/01 - cpu execution"
~~~

from the repository root.

### The Program Runs but Prints Nothing

Run `./build.sh 1` to select the first experiment. Running `./build.sh` without a selection displays the menu. If you changed a runner, make sure it still calls its computation and prints the results.

### Timings Change Between Runs

Some variation is normal. Repeat the measurements, reduce heavy background activity, document your summary method, and focus on patterns rather than one isolated number.

### VS Code Shows Red Underlines but cc Compiles Successfully

VS Code's code-analysis configuration can differ from the actual terminal compiler. The strict terminal build is the final authority for this lab. You can still correct VS Code's include-path settings later.

### git push Asks for Authentication

Follow GitHub's browser or credential-manager prompts. GitHub does not accept an account password as a command-line Git password over HTTPS.

### The Pull Request Targets the Wrong Repository

Before submitting, recheck:

~~~text
base: rvfvn/blackhole, branch main
compare: YOUR-USERNAME/blackhole, branch lab01-YOUR-USERNAME
~~~

## 22. Final Submission Checklist

Before requesting review, confirm all of the following:

- Your fork was synchronized before you created the work branch.
- Your branch contains your individual implementation.
- Every required source function is implemented.
- Each lab file compiles and runs independently through build.sh.
- The project builds with the documented build.sh command.
- Compiler errors are gone and warnings have been investigated.
- Results are checked for correctness.
- Timing setup is described.
- Machine information is included.
- Terminal output is included in the pull-request description.
- Your analysis explains the results rather than only listing numbers.
- Assembly observations are included.
- Generated executables and assembly files are not committed.
- The PR targets rvfvn/blackhole main.
- The PR is ready for instructor review.

The final goal is not for every student to produce identical code or identical timing. The goal is for each student to produce a correct, explainable experiment and enough evidence for the class to understand how source code, machine execution, data movement, and parallel work relate.

## References and Source Notes

The first group below records sources that are cited directly in this guide or in the Session 1 slides. The later groups contain authoritative references that support the technical explanations in the starter code and lab workflow. Those supporting references were reconstructed from the subjects covered by the lab; they should not be interpreted as a claim that they are the exact pages originally consulted while every comment was written.

### Sources Cited Directly in the Lab or Slides

- [Session 1 slide deck](./CPU-Execution-Lab-Session-1.pptx.pdf)
- [University of Wisconsin-Madison CS 354: Machine Organization Basics](https://pages.cs.wisc.edu/~markhill/cs354/Fall2008/notes/basics.html) - instructions, registers, memory reads and writes, and the basic stored-program model.
- [Cornell CS 3410: Caches](https://www.cs.cornell.edu/courses/cs3410/2024fa/notes/caches.html) - cache hierarchy, cache lines, locality, and memory-access behavior.
- [Cornell CS 3410: Pipelining and CPU Performance](https://www.cs.cornell.edu/courses/cs3410/2025sp/notes/pipelining.html) - instruction execution and CPU performance.
- [Wikipedia: Central processing unit](https://en.wikipedia.org/wiki/Central_processing_unit) - introductory CPU description used on the CPU slide.
- [GeeksforGeeks: Central Processing Unit](https://www.geeksforgeeks.org/computer-science-fundamentals/central-processing-unit-cpu/) - CPU components and the fetch, decode, execute, and store explanation and image used in the slides.
- [Algorithmica: External Memory](https://en.algorithmica.org/hpc/external-memory/hierarchy/) - memory-hierarchy illustration used in the slides.
- [DataChant: Excel Matrix Multiplication](https://datachant.com/2016/06/01/excel-matrix-multiplication-replacing-mmult-with-power-query/) - matrix-multiplication example cited on the slides.
- [GeeksforGeeks: How to Multiply 2 x 2 Matrices](https://www.geeksforgeeks.org/maths/how-to-multiply-2-x-2-matrices/) - introductory matrix-multiplication example cited on the slides.
- [Pinterest: Matrix Multiplication Image](https://ru.pinterest.com/pin/477029785519648105/) - image provenance recorded in the slides; this is not used as a technical authority.

### Setup, Git, and Development Environment

- [GitHub: Work with forks](https://docs.github.com/en/pull-requests/how-tos/work-with-forks) - the relationship between the class repository and each student's fork.
- [GitHub: Syncing a fork](https://docs.github.com/en/pull-requests/how-tos/work-with-forks/syncing-a-fork) - bringing upstream changes into a fork.
- [GitHub: Cloning a repository](https://docs.github.com/en/repositories/creating-and-managing-repositories/cloning-a-repository) - creating the local clone.
- [GitHub: Creating a pull request](https://docs.github.com/en/pull-requests/how-tos/create-pull-requests/creating-a-pull-request) - submitting individual implementations and lab reports for review.
- [Visual Studio Code](https://code.visualstudio.com/) - editor download.
- [Visual Studio Code: Terminal Basics](https://code.visualstudio.com/docs/terminal/basics) - running the lab commands in the integrated terminal.
- [Visual Studio Code: Developing in WSL](https://code.visualstudio.com/docs/remote/wsl) - opening the Linux-side repository correctly on Windows.
- [Microsoft: Set Up a WSL Development Environment](https://learn.microsoft.com/en-us/windows/wsl/setup/environment) - installing and configuring Ubuntu through WSL.
- [Apple: Installing the Command-Line Tools](https://developer.apple.com/documentation/xcode/installing-the-command-line-tools) - installing Apple Clang and command-line development tools on macOS.
- [Ubuntu: build-essential Package](https://packages.ubuntu.com/search?keywords=build-essential) - the compiler and standard build-tool package used by Ubuntu and WSL setup.

### C, POSIX, and Compiler Behavior

- [cppreference: C Arrays](https://en.cppreference.com/c/language/array) - contiguous array elements, multidimensional arrays, and row-major layout.
- [The Open Group: clock_gettime and CLOCK_MONOTONIC](https://pubs.opengroup.org/onlinepubs/000095399/functions/clock_getres.html) - monotonic timestamps and `struct timespec` timing.
- [GNU C Library: Feature Test Macros](https://sourceware.org/glibc/manual/latest/html_node/Feature-Test-Macros.html) - why `_POSIX_C_SOURCE 200809L` exposes POSIX.1-2008 interfaces on systems using glibc.
- [The Open Group: sysconf](https://pubs.opengroup.org/onlinepubs/9699919799/functions/sysconf.html) - querying operating-system configuration values.
- [Linux man-pages: sysconf](https://man7.org/linux/man-pages/man3/sysconf.3.html) - the `_SC_NPROCESSORS_ONLN` query used by `machine_info()` in lib/bench.c.
- [GCC: Overall Options](https://gcc.gnu.org/onlinedocs/gcc/Overall-Options.html) - compilation stages and producing assembly with `-S`.
- [GCC: Optimize Options](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html) - transformations enabled by optimization levels, including loop and basic-block vectorization at `-O2`.
- [Clang Command Guide](https://clang.llvm.org/docs/CommandGuide/clang.html) - Clang compilation stages, `-S`, language-standard selection, and optimization levels.
- [David Goldberg: What Every Computer Scientist Should Know About Floating-Point Arithmetic](https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html) - rounding behavior and why equivalent accumulation orders can differ slightly.

### Memory Access and Numerical Performance

- [Intel 64 and IA-32 Architectures Optimization Reference Manual, Volume 1](https://cdrdv2-public.intel.com/821612/248966-Optimization-Reference-Manual-V1-050.pdf) - cache locality, cache-line traffic, prefetching, and the performance cost of large-stride access.
- [Netlib BLAS: GEMM](https://www.netlib.org/lapack/explore-html/dd/d09/group__gemm.html) - the standard general matrix-matrix multiplication operation.
- [UT Austin LAFF: Matrix-Matrix Multiplication](https://www.cs.utexas.edu/~flame/laff/laff/LAFF.pdf) - matrix multiplication, data reuse, and the approximately `2n^3` floating-point-operation count.
- [Berkeley Lab: Roofline Performance Model](https://amcr.lbl.gov/departments/computer-science-department/ppan/roofline-performance-model/) - the relationship among computation, memory traffic, bandwidth, and attainable performance.

### Tenstorrent and Blackhole

- [Tenstorrent: Installing Blackhole PCIe Cards](https://docs.tenstorrent.com/aibs/blackhole/installation.html) - Blackhole installation material cited in the original lab introduction.
- [Tenstorrent: Blackhole PCIe Cards](https://docs.tenstorrent.com/aibs/blackhole/) - current Blackhole card specifications, Tensix-core count, SRAM, RISC-V cores, GDDR6 capacity, and memory bandwidth.
- [TT-Metalium: Memory from a Kernel Developer's Perspective](https://docs.tenstorrent.com/tt-metal/latest/tt-metalium/tt_metal/advanced_topics/memory_for_kernel_developers.html) - shared on-core SRAM, private RISC-V memory, and the memory visible to kernels.
- [TT-Metalium Lab 1: Single-Core Matrix Multiplication](https://docs.tenstorrent.com/tt-metal/latest/tt-metalium/tt_metal/labs/matmul/lab1/lab1.html) - Tensix cores, the NoC, local SRAM, DRAM movement, and a device matrix-multiplication example.
- [TT-Metalium: Data Movement APIs](https://docs.tenstorrent.com/tt-metal/latest/tt-metalium/tt_metal/apis/kernel_apis/data_movement/data_movement.html) - explicit NoC reads, writes, multicast, barriers, and local-memory operations.
- [TT-Metalium: Compute Engines and Data Flow within Tensix](https://docs.tenstorrent.com/tt-metal/latest/tt-metalium/tt_metal/advanced_topics/compute_engines_and_dataflow_within_tensix.html) - RISC-V control, data flow between SRAM and registers, and specialized matrix and vector engines.
