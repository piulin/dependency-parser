from models import model
from parsers.arcstandard import Parser

class Model(model.Model):

    def __init__(self, trans):
        self.trans_ = trans
        self.scores_ = [ [ 1 if tr == Parser.Stc_parser.shift else 0 ,
                           1 if tr == Parser.Stc_parser.leftarc else 0 ,
                           1 if tr == Parser.Stc_parser.rightarc else 0 ] for tr in self.trans_ ]
        self.counter_ = 0


    def score_transitions(self, tk1, tk2):
        score = self.scores_ [  self.counter_  ]
        self.counter_ += 1
        return score