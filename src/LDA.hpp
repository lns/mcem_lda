#pragma once

#include "DenseMat.hpp"
#include "SparseMat.hpp"
#include "AliasUrn.hpp"
#include "Heap.hpp"
#include <vector>
#include <algorithm>
#include <string>
#include "py_util.hpp"

using std::vector;

#define ASSERT(x) assert(x)

class LDA {
public:
	// Types
#if 1
	typedef uint16_t topic_index_t;
	typedef uint32_t doc_index_t;
	typedef uint32_t word_index_t;
	typedef uint64_t token_index_t;
#else
	typedef int topic_index_t;
	typedef int doc_index_t;
	typedef int word_index_t;
	typedef int token_index_t;
#endif

	struct Z {
	public:
		topic_index_t old_z;
		topic_index_t new_z;
		Z(topic_index_t o, topic_index_t n) : old_z(o), new_z(n) {}
	};

	// Parameters
	topic_index_t n_topic;
	float alpha, beta, alpha_bar, beta_bar;
	// Objects
	// These could be DenseRowMat. See DenseMat.hpp for details.
	DenseMat<int> C_doc;
	DenseMat<int> C_word;
	vector<int> C_all; // C_all[k] = #token in topic k
	SparseMat<Z,token_index_t,doc_index_t,word_index_t> corpus;
	qlib::XOR128PLUS rng;

	topic_index_t get_n_topic() const { return n_topic; }
	void set_n_topic(topic_index_t n) {
		n_topic = n;
		alpha_bar = n*alpha;
		beta_bar = n*beta;
	}
	float get_alpha() const { return alpha; }
	void set_alpha(float a) {
		alpha = a;
		alpha_bar = n_topic*a;
	}
	float get_beta() const { return beta; }
	void set_beta(float b) {
		beta = b;
		beta_bar = n_topic*b;
	}

	void check() {
		// Count all
		auto cnt = 0;
		for(const auto& each : C_all)
			cnt += each;
		ASSERT(cnt == corpus.nnz());
		cnt = 0;
		for(int i=0; i<C_doc.nrow(); i++)
			for(int j=0; j<C_doc.ncol(); j++)
				cnt += C_doc.at(i,j);
		ASSERT(cnt == corpus.nnz());
		cnt = 0;
		for(int i=0; i<C_word.nrow(); i++)
			for(int j=0; j<C_word.ncol(); j++)
				cnt += C_word.at(i,j);
		ASSERT(cnt == corpus.nnz());
		// Count Ld
		for(int d=0; d<C_doc.nrow(); d++) {
			cnt = 0;
			for(int j=0; j<C_doc.ncol(); j++)
				cnt += C_doc.at(d,j);
			auto bgn = corpus.csr_index_[d];
			auto end = corpus.csr_index_[d+1];
			ASSERT(cnt == end - bgn);
		}
		// Count Lw
		for(int w=0; w<C_word.nrow(); w++) {
			cnt = 0;
			for(int j=0; j<C_word.ncol(); j++)
				cnt += C_word.at(w,j);
			auto bgn = corpus.csc_index_[w];
			auto end = corpus.csc_index_[w+1];
			ASSERT(cnt == end - bgn);
		}
	}

	// Methods
	void init() {
		C_doc.resize(corpus.nrow(), n_topic);
		C_word.resize(corpus.ncol(), n_topic);
		C_all.resize(n_topic);
		// Fill z randomly
		corpus.apply([&](Z& z, doc_index_t d, word_index_t w) {
			z.old_z = rng.sample() % n_topic;
			z.new_z = rng.sample() % n_topic;
		});
		// Update C_doc & C_all
		C_doc.clear();
		corpus.apply([&](Z& z, doc_index_t d, word_index_t w) {
			C_doc.at(d, z.old_z) ++;
			C_all[z.old_z] ++;
		});
		// Update C_word
		C_word.clear();
		corpus.apply<true>([&](Z& z, doc_index_t d, word_index_t w) {
			C_word.at(w, z.old_z) ++;
		});
	}

	// Accept and update by word
	void sample_by_word() {
		const auto n_word = corpus.ncol();
		for(auto w = 0; w < n_word; w++) {
			const token_index_t bgn = corpus.csc_index_[w];
			const token_index_t end = corpus.csc_index_[w+1];
			// 0.1 Update C_word
			for(auto t=0; t<n_topic; t++)
				C_word.at(w, t) = 0;
			for(auto i=bgn; i<end; i++) {
				Z& z = corpus.val_[corpus.csc_val_index_[i]];
				C_word.at(w, z.old_z) ++;
			}
			// 1. Accept new_z and update C_all on the fly
			for(auto i=bgn; i<end; i++) {
				Z& z = corpus.val_[corpus.csc_val_index_[i]];
				// accept and update
				if(z.new_z == z.old_z)
					continue;
				double accept_prob = 
					(C_word.at(w, z.new_z) + beta)/(C_word.at(w, z.old_z) + beta) *
					(C_all[z.old_z] + beta_bar)/(C_all[z.new_z] + beta_bar);
				if(rng.drand() < accept_prob) {
					C_word.at(w, z.new_z) ++;
					C_word.at(w, z.old_z) --;
					C_all[z.new_z] ++;
					C_all[z.old_z] --;
					z.old_z = z.new_z;
				}
			}
			// 2. Draw new_z
			AliasUrn<decltype(rng)> urn(rng);
			vector<double> prob;
			for(topic_index_t t=0; t<n_topic; t++)
				prob.push_back(C_word.at(w,t) + beta);
			urn.setup(prob);
			for(token_index_t i=bgn; i<end; i++) {
				Z& z = corpus.val_[corpus.csc_val_index_[i]];
				// draw
				z.new_z = urn.sample();
			}
		}
	}

	// Accept and update by doc
	void sample_by_doc() {
		const auto n_doc = corpus.nrow();
		for(auto d = 0; d < n_doc; d++) {
			const token_index_t bgn = corpus.csr_index_[d];
			const token_index_t end = corpus.csr_index_[d+1];
			// 0. Update C_word
			for(auto t=0; t<n_topic; t++)
				C_doc.at(d, t) = 0;
			for(auto i=bgn; i<end; i++) {
				Z& z = corpus.val_[i];
				C_doc.at(d, z.old_z) ++;
			}
			// 1. Accept new_z and update C_all on the fly
			for(auto i=bgn; i<end; i++) {
				Z& z = corpus.val_[i];
				// accept and update
				if(z.new_z == z.old_z)
					continue;
				double accept_prob =
					(C_doc.at(d, z.new_z) + alpha)/(C_doc.at(d, z.old_z) + alpha) *
					(C_all[z.old_z] + beta_bar)/(C_all[z.new_z] + beta_bar);
				if(rng.drand() < accept_prob) {
					C_all[z.new_z] ++;
					C_all[z.old_z] --;
					z.old_z = z.new_z;
				}
			}
			// 2. Draw new_z
			const unsigned Ld = end - bgn;
			const double position_sample_prob = Ld/(Ld + alpha_bar);
			for(token_index_t i=bgn; i<end; i++) {
				Z& z = corpus.val_[i];
				// draw
				if(rng.drand() < position_sample_prob)
					z.new_z = corpus.val_[bgn + (rng.sample() % Ld)].old_z;
				else
					z.new_z = rng.sample() % n_topic;
			}
		}
	}

	// Get Top Words
	// Todo: get top words for all topics with one pass
	std::vector<std::pair<word_index_t,int>>
	get_top_words(topic_index_t topic, size_t max_size = 20) const {
		typedef std::pair<word_index_t,int> T;
		class Comp {
		public:
			bool operator()(const T& a, const T& b) {
				return a.second > b.second;
			}
		};
		Heap<T,Comp> heap;
		heap.set_max_size(max_size);
		const auto n_word = corpus.ncol();
		for(auto w = 0; w < n_word; w++)
			heap.push(std::make_pair(w,C_word.at(w,topic)));
		heap.sort();
		return heap.get();
	}

	py::list py_get_top_words(topic_index_t topic, size_t max_size = 20) const {
		auto vec = get_top_words(topic, max_size);
		py::list l;
		for(const auto& each : vec)
			l.append(each.first);
		return l;
	}

	void mcem(size_t n_iter, bool verbose = false) {
		for(auto i=0; i<n_iter; i++) {
			sample_by_word();
			sample_by_doc();
			if(verbose)
				printf("iter %3d: %le\n", i, pseudo_loglikelihood());
		}
	}

	double pseudo_loglikelihood() { // indeed const
		double res = 0;
		corpus.apply<true>([&](Z& z, doc_index_t d, word_index_t w) {
			// Todo: This could be optimized with a HashMap
			res += log(C_word.at(w, z.old_z) + beta);
		});
		for(topic_index_t k=0; k<n_topic; k++)
			res -= C_all[k]*log(C_all[k] + beta_bar);
		return res;
	}

	void py_read_corpus(py::list l) {
		for(int r=0; r<len(l); r++) {
			py::list doc = py::extract<py::list>(l[r]);
			for(int j=0; j<len(doc); j++) {
				py::tuple t = py::extract<py::tuple>(doc[j]);
				unsigned idx = py::extract<unsigned>(t[0]);
				unsigned cnt = py::extract<unsigned>(t[1]);
				for(int k=0; k<cnt; k++)
					corpus.append(r,idx,0,0);
			}
		}
		corpus.build_CSC_from_CSR();
	}

	void print() const {
		printf("C_all: ");
		for(const auto& each : C_all)
			printf("%u ",each);
		printf("\n");
		printf("C_doc: \n");
		for(int i=0; i<std::min<int>(5,corpus.nrow()); i++) {
			for(int j=0; j<std::min<int>(5,n_topic); j++) {
				printf("%u ", C_doc.at(i,j));
			}
			printf("\n");
		}
		printf("C_word: \n");
		for(int i=0; i<std::min<int>(5,corpus.ncol()); i++) {
			for(int j=0; j<std::min<int>(5,n_topic); j++) {
				printf("%u ", C_word.at(i,j));
			}
			printf("\n");
		}
	}

	void write2csv(const std::string& prefix) const {
		if(true) { // C_word
			FILE * f = fopen(std::string(prefix+"_Cword.csv").c_str(), "w");
			for(word_index_t w = 0; w<C_word.nrow(); w++) {
				for(topic_index_t t = 0; t<C_word.ncol(); t++) {
					if(t!=0)
						fprintf(f,",");
					fprintf(f,"%d",C_word.at(w,t));
				}
				fprintf(f,"\n");
			}
			fclose(f);
		}
		if(true) { // C_doc
			FILE * f = fopen(std::string(prefix+"_Cdoc.csv").c_str(), "w");
			for(doc_index_t d = 0; d<C_doc.nrow(); d++) {
				for(topic_index_t t = 0; t<C_doc.ncol(); t++) {
					if(t!=0)
						fprintf(f,",");
					fprintf(f,"%d",C_doc.at(d,t));
				}
				fprintf(f,"\n");
			}
			fclose(f);
		}
	}

};

