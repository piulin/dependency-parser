# Summary

The C++ code in this repository implements an averaged perceptron and a naïve Chu-Liu-Edmonds decoder to learn and predict labeled dependency parses from CONLL06 annotated data.

## Installation

Build the makefile for your own architecture:
```
$ mkdir build
$ cd build
$ cmake ..
```
Then compile the source code:
```
$ make
```
The executable ```c__``` should pop up in the ```build``` directory.

## How to use

Depending on the action you wanna perform, namely training or test, you would have pass to the program a different set of arguments.

### Training

Syntax:

```
c__ -t <training_dataset>     conll06 file used for learning the perceptron.
[-f <dev_dataset>]            conll06 development dataset for reporting accuracies at each epoch.
[-a <test_dataset>]           conll06 test file.
[-e <#ephocs>]                sets the number of ephocs.
[-d <model>]                  dumps the learnt model into a file.
[-o <output_conll>]           outputs a conll file with the predicted dependencies.
```
### Test

```
c__ -a <test_dataset>         conll06 test file.
-r <model>                    loads up the perceptron weights (learnt during training) from a file.
```

# REPORT

The purpose of this short report is to briefly review the parser built
over the past three months for the subject “Statistical Dependency
Parsing". In it, an overview of the most relevant aspects
of the parser such as the decoder algorithm, the feature model and
machine learning algorithm is given.

#### Chu-Liu-Edmonds.

The Chu-Liu-Edmonds is an algorithm able to find the Maximum Spanning
Tree (MST) of a weighted directed graph. In dependency parsing, a
correct dependency tree of a sentence can be obtained by finding the MST
of a weighted directed graph whose weights have been appropriately set.
For the representation of the graph, no external libraries have been
used. The internal representation is based in two matrices of size
proportional to *n* (the number of tokens of a sentence): the adjacency
matrix *A* ∈ ℳ<sub>*n* × *n*</sub>(𝔹) and the cost matrix
*C* ∈ ℳ<sub>*n* × *n* − 1</sub>(ℤ) which express the adjacency and
weights of each par of nodes, respectively. Chu-Liu-Edmonds builds the
MST by greedily connecting pairs of nodes with the highest incoming
weight and removing the cycles in a recursive fashion, until no more are
found. The problem of finding the cycles is addressed by taking
advantage of an interesting property of adjacency matrices: the element
(*i*, *j*) of the matrix *A*<sup>*p*</sup> gives the number of paths of
size *p* from the node *i* to the node *j*. To find a cycle it is
sufficient to calculate *A*<sup>*p*</sup> ∀*p* = 2, …, *n* and check if
non-zero values appear in the diagonal, called candidates. The nodes
that a loop consist of can be retrieved by searching the path that
connects one of the candidates to itself following a path of size *p*
through the remaining candidates. In relation to the reminder operations
of the algorithm, namely `contract` and `solve_cycle`, they are
performed by shrinking or expanding the matrices *A* and *C* and
accordingly modifying their values.

#### Features.

The feature set is comprised by the templates proposed in [1] and some
features from [2]: `hform:dform:hpos`, `hpos:hpos+1:dpos`,
`hpos:dpos-1:dpos` and `hpos:dpos:dpos+1`. Additionally, in order to
reduce the sparsity of the data, the feature describing the distance
between tokens *d* is modified so that only it can take only four
discrete values: itself if *d* &lt; 4 or an additional identifier if
*d* ≥ 4. The token form of the numbers is normalized so that all the
numbers are redirected to the same feature identifier and, on the other
hand, the form of the reminder tokens are lowercased. In relation to the
internal representation of the features, the pairs feature-identifier
are kept in *l* different hash tables (one for each template) whose
purpose is nothing but reduce the expensive string concatenation to the
maximum extent possible. In this sense, the concatenation of the
different elements a template consists of is done without any separator
symbol. It is also worth noting that, for the sake of efficiency, the
features are only added to the hash tables if while training, a
dependency tree was incorrectly predicted. Furthermore, to avoid the
sparsity on the weight vectors used in the perceptron, only those
features corresponding to the gold standard and the incorrect tree are
included in the tables.

#### Averaged Perceptron.

The Machine Learning algorithms used to predict (1) the dependencies
between tokens and (2) predict the labels of the dependencies are an
averaged perceptron and a multi-class perceptron, respectively. The
former, whose only purpose is to feed cost matrices, is represented as
two matrices *W*<sub>*D*</sub> ∈ ℳ<sub>1 × *m*</sub>(ℤ) and
*Q* ∈ ℳ<sub>1 × *m*</sub>(ℤ) that keep track of the weights and the
cached weights. Similarly, the multi-class perceptron[1] used for label
prediction is represented as a matrix
*W*<sub>*L*</sub> ∈ ℳ<sub>*l* × *m*</sub>(ℤ) where *l* is the number of
different labels appearing in the corpus. During the first epoch of a
training session, the size *m* of matrices *W*<sub>*D*</sub>, *Q* and
*W*<sub>*L*</sub> is dynamically increased in chunks of size *c* = 500
(in order to allocate the new features generated) until all the input is
consumed. The learning approach consists in increasing and decreasing
unitary values in *W*<sub>*D*</sub> and *W*<sub>*L*</sub> according to
the features describing the gold standard and the predicted dependency
trees, respectively. This update operation is only performed when the
predicted tree mismatchs the gold standard. It is also worth noting that
besides the average perceptron, the set of training sentences is
shuffled in each epoch to minimize the effect of the last updates in the
weight vectors.

#### Quick note on high performance.

Avoiding the use of external libraries was a good choice as long as the
performance is concerned. The code has been parallelized using OpenMP to
take advantage of all available processors. Many operations in the
Chu-Liu-Edmonds decoder e.g. finding cycles, the prediction of
dependency trees while training, the dot product in the perceptron, and
the extraction of features constitute some examples of parallel sections
in the parser that improve computational times even further.

Experiments
===========

In this section, the experiments accomplished to improve the accuracies
for both English and German datasets are shown.

#### Experimental setup.

Since just a small corpora is sufficient to test behavior of the code
modifications proposed in this section, the experiments have been
carried out using the provided 5k training corpus for each language. The
accuracies are measured in terms of the UAS and LAS achieved in the
development datasets. On the other hand, the code was compiled by
`g++ 9.2.0` with aggressive optimizations and the experiments were
executed in a workstation consisting of 16 GB of RAM, a 9th generation
Intel Core i7 @ 4.5 GHz and a 64-bit Arch Linux.

#### Baseline.

In order to measure the accuracy improvement of the modifications, a
configuration consisting of the Chu-Liu-Edmonds decoder and a regular
(non-averaged) 5-epoch perceptron fed just with the features of is
proposed as a baseline.

<table>
<caption>Experimental results. Agv. perceptron, Dist. reduction, Add. templates and Ex. times stand for Averaged perceptron, Distance reduction, Additional templates and Execution times, respectively.</caption>
<tbody>
<tr class="odd">
<td style="text-align: left;">Dataset</td>
<td style="text-align: right;">English</td>
<td style="text-align: right;">English</td>
<td style="text-align: right;">English</td>
<td style="text-align: right;">German</td>
<td style="text-align: right;">German</td>
<td style="text-align: right;">German</td>
</tr>
<tr class="even">
<td style="text-align: left;"></td>
<td style="text-align: right;"><em>UAS (%)</em></td>
<td style="text-align: right;"><em>LAS (%)</em></td>
<td style="text-align: right;"><em>Times (s)</em></td>
<td style="text-align: right;"><em>UAS (%)</em></td>
<td style="text-align: right;"><em>LAS (%)</em></td>
<td style="text-align: right;"><em>Times (s)</em></td>
</tr>
<tr class="odd">
<td style="text-align: left;">Baseline</td>
<td style="text-align: right;">81.16</td>
<td style="text-align: right;">79.72</td>
<td style="text-align: right;">35.75</td>
<td style="text-align: right;">84.00</td>
<td style="text-align: right;">80.27</td>
<td style="text-align: right;">21.71</td>
</tr>
<tr class="even">
<td style="text-align: left;">+Avg. perceptron</td>
<td style="text-align: right;">82.91</td>
<td style="text-align: right;">81.39</td>
<td style="text-align: right;">33.43</td>
<td style="text-align: right;">84.93</td>
<td style="text-align: right;">81.15</td>
<td style="text-align: right;">20.88</td>
</tr>
<tr class="odd">
<td style="text-align: left;">+Dist. reduction</td>
<td style="text-align: right;">84.90</td>
<td style="text-align: right;">83.36</td>
<td style="text-align: right;"><strong>28.93</strong></td>
<td style="text-align: right;">86.88</td>
<td style="text-align: right;">83.15</td>
<td style="text-align: right;">18.53</td>
</tr>
<tr class="even">
<td style="text-align: left;">+Normalization</td>
<td style="text-align: right;">84.91</td>
<td style="text-align: right;">83.46</td>
<td style="text-align: right;">28.96</td>
<td style="text-align: right;">86.93</td>
<td style="text-align: right;">83.33</td>
<td style="text-align: right;"><strong>18.21</strong></td>
</tr>
<tr class="odd">
<td style="text-align: left;">+Add. templates</td>
<td style="text-align: right;">85.38</td>
<td style="text-align: right;">83.85</td>
<td style="text-align: right;">34.69</td>
<td style="text-align: right;">87.18</td>
<td style="text-align: right;">83.55</td>
<td style="text-align: right;">21.31</td>
</tr>
<tr class="even">
<td style="text-align: left;">+Shuffle</td>
<td style="text-align: right;"><span class="math inline"><strong>85.39</strong> ± <strong>0.09</strong></span></td>
<td style="text-align: right;"><span class="math inline"><strong>83.88</strong> ± <strong>0.12</strong></span></td>
<td style="text-align: right;"><span class="math inline">34.58 ± 0.67</span></td>
<td style="text-align: right;"><span class="math inline"><strong>87.41</strong> ± <strong>0.08</strong></span></td>
<td style="text-align: right;"><span class="math inline"><strong>83.65</strong> ± <strong>0.13</strong></span></td>
<td style="text-align: right;"><span class="math inline">22.77 ± 1.30</span></td>
</tr>
</tbody>
</table>

In the Table
<a href="#tab:results" data-reference-type="ref" data-reference="tab:results">1</a>
are detailed the accuracies and execution times yielded for each
modification of the code in relation to the baseline. The results of
such modifications are incremental, this meaning that each modification
also carries all the previous ones (in the table from the bottom to the
top). As far as the results are concerned, the baseline achieves fair
accuracies both for German (84.00 %) and English (81.16 %) datasets,
higher than the 75 % of UAS requested to pass this course. The
implementation of the average perceptron (Avg. perceptron) seems to have
a little positive impact in the German dataset (+0.93 % UAS, +0.88 %
LAS) and a fair impact in the English dataset (+1.75 UAS, +1.67 LAS),
probably due to the fact that the English dataset is bigger than the
German in terms of number of tokens, thus the effect of averaging could
be more pronounced. The reduction in the sparsity of the data by
accounting only four kinds of distance between tokens (Dist. reduction)
has a very positive impact in both datasets: +1.99 % UAS, +1.97 and
+1.95 % UAS, +2.00 % LAS for the English and German datasets,
respectively. On the contrary, the normalization in the form of the
tokens (lowercase and numbers substitution) have almost no impact in
English dataset and a slighter larger influence in the German one,
probably because the capitalization in the German language plays a more
relevant role than in English. Then, the inclusion of new templates
(Add. templates) from caused the accuracies to rise another bit in both
models (English: +0.47 % UAS, +0.39 % LAS; German: +0.25 % UAS, +0.22 %
LAS) whilst degrading significantly the computational time of the
algorithm. Finally, I was curious to check out whether the shuffling of
the sentences fed to the averaged perceptron in each epoch could affect
positively (both methods try to minimize same effect) the accuracies or
not. To do so, ten executions including shuffling were carried out. The
resulting mean and standard deviation can be seen in the last row of the
Table
<a href="#tab:results" data-reference-type="ref" data-reference="tab:results">1</a>:
in average the shuffling has a very subtle but positive impact in both
English and German datasets with no influence in the execution times
whatsoever.  
Once knowing that these extensions improved the accuracies of the
perceptron, the training can be performed with the whole training set.
However, the number of epochs that was previously selected using the
rule of thumb had to be first characterized.

<table>
<caption>UAS obtained for the English dataset varying the number of epochs from 1 to 14. The experiments were repeated 4 times each.</caption>
<thead>
<tr class="header">
<th style="text-align: right;"><strong># Epochs</strong></th>
<th style="text-align: right;"><strong>UAS</strong> % (<span class="math inline"><em>μ</em> ± <em>σ</em></span>)</th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td style="text-align: right;">1</td>
<td style="text-align: right;"><span class="math inline">87.88 ± 0.19</span></td>
</tr>
<tr class="even">
<td style="text-align: right;">2</td>
<td style="text-align: right;"><span class="math inline">88.48 ± 0.11</span></td>
</tr>
<tr class="odd">
<td style="text-align: right;">3</td>
<td style="text-align: right;"><span class="math inline">88.60 ± 0.06</span></td>
</tr>
<tr class="even">
<td style="text-align: right;">4</td>
<td style="text-align: right;"><span class="math inline">88.66 ± 0.06</span></td>
</tr>
<tr class="odd">
<td style="text-align: right;">5</td>
<td style="text-align: right;"><span class="math inline">88.69 ± 0.08</span></td>
</tr>
<tr class="even">
<td style="text-align: right;">6</td>
<td style="text-align: right;"><span class="math inline">88.77 ± 0.06</span></td>
</tr>
<tr class="odd">
<td style="text-align: right;">7</td>
<td style="text-align: right;"><span class="math inline">88.77 ± 0.10</span></td>
</tr>
<tr class="even">
<td style="text-align: right;"><strong>8</strong></td>
<td style="text-align: right;"><span class="math inline"><strong>88.79</strong> ± <strong>0.03</strong></span></td>
</tr>
<tr class="odd">
<td style="text-align: right;">9</td>
<td style="text-align: right;"><span class="math inline">88.74 ± 0.11</span></td>
</tr>
<tr class="even">
<td style="text-align: right;">10</td>
<td style="text-align: right;"><span class="math inline">88.79 ± 0.09</span></td>
</tr>
<tr class="odd">
<td style="text-align: right;">11</td>
<td style="text-align: right;"><span class="math inline">88.82 ± 0.10</span></td>
</tr>
<tr class="even">
<td style="text-align: right;">12</td>
<td style="text-align: right;"><span class="math inline">88.77 ± 0.10</span></td>
</tr>
<tr class="odd">
<td style="text-align: right;">13</td>
<td style="text-align: right;"><span class="math inline">88.67 ± 0.06</span></td>
</tr>
<tr class="even">
<td style="text-align: right;">14</td>
<td style="text-align: right;"><span class="math inline">88.75 ± 0.11</span></td>
</tr>
</tbody>
</table>

Table
<a href="#tab:tal" data-reference-type="ref" data-reference="tab:tal">2</a>
presents the accuracies varying the number of training epochs using the
whole english dataset. Clearly, the number of epochs that seems most
appropriate for the english dataset is 8 if we account for a trade-off
between computational time, UAS average, and UAS standard deviation. The
same procedure has been done to select the number of epochs (7) for the
German dataset. To conclude this section, the Table
<a href="#tab:final" data-reference-type="ref" data-reference="tab:final">3</a>
summarizes the accuracies achieved for the test and development
datasets.

<table>
<caption>Comparison to the results achieved in the test and development datasets.</caption>
<tbody>
<tr class="odd">
<td style="text-align: right;"><em>Test</em></td>
<td style="text-align: right;"><em>Dev</em></td>
<td style="text-align: right;"><em>Test</em></td>
<td style="text-align: right;"><em>Dev</em></td>
<td style="text-align: right;"><em>Test</em></td>
<td style="text-align: right;"><em>Dev</em></td>
<td style="text-align: right;"><em>Test</em></td>
<td style="text-align: right;"><em>Dev</em></td>
</tr>
<tr class="even">
<td style="text-align: right;">90.01</td>
<td style="text-align: right;">88.80</td>
<td style="text-align: right;">88.97</td>
<td style="text-align: right;">87.35</td>
<td style="text-align: right;">88.24</td>
<td style="text-align: right;">91.03</td>
<td style="text-align: right;">85.09</td>
<td style="text-align: right;">88.26</td>
</tr>
</tbody>
</table>

Conclusions
===========

In this project a graph-based dependency parser has been developed in
C++ without the use of any external library. It features the
Chu-Liu-Edmonds decoder able to produce non-projective parses, an
average perceptron for the classification of dependencies, and a
multi-class perceptron for the classification of the dependency labels.
The features used to train the model are the unigram and bigram
templates proposed in . A number of experiments have been carried out
showing the performance improvement of the included extensions whilst
the computational times have been kept in reasonable numbers by using
code optimizations and introducing parallel regions within the code.

[1] Ryan McDonald, Fernando Pereira, Kiril Ribarov, and Jan Hajič. Non-projective depen-
dency parsing using spanning tree algorithms. 01 2005.

[2] Bernd Bohnet. Very high accuracy and fast dependency parsing is not a contradiction. In
Proceedings of the 23rd International Conference on Computational Linguistics, COLING
’10, page 89–97, USA, 2010. Association for Computational Linguistics.

Shield: [![CC BY 4.0][cc-by-shield]][cc-by]

This work is licensed under a
[Creative Commons Attribution 4.0 International License][cc-by].

[![CC BY 4.0][cc-by-image]][cc-by]

[cc-by]: http://creativecommons.org/licenses/by/4.0/
[cc-by-image]: https://i.creativecommons.org/l/by/4.0/88x31.png
[cc-by-shield]: https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg
