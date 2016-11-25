#!/System/Library/Frameworks/Python.framework/Versions/2.7/bin/python
# -*- coding: utf-8 -*-
import mcemlda
import cPickle
nips = cPickle.load(open("nips.cPickle","r"))
print(len(nips['corpus']))
lda = mcemlda.LDA()
lda.read_corpus(nips['corpus'])
lda.n_topic = 64
lda.alpha = 50.0/lda.n_topic
lda.beta = 0.01
lda.init()
lda.show()
lda.mcem(300, True)
lda.show()
lda.write2csv("lda_model")
vocab_plain = nips['vocab_plain']
for i in range(lda.n_topic):
    print ' ======== ' + str(i) + ' ======== '
    print " ".join([vocab_plain[idx] for idx in lda.get_top_words(i,20)])

