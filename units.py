class Token:
    def __init__(self, form, lemma, pos, xpos, morph, head, rel):
        self.form_ = form
        self.lemma_ = lemma
        self.pos_ = pos
        self.xpos_ = xpos
        self.morph_ = morph
        self.head_ = head
        self.rel_ = rel

    def __init__ (self, token_stream : str ):
        self.form_ = ""
        self.lemma_ = ""
        self.pos_ = ""
        self.xpos_ = ""
        self.morph_ = ""
        self.head_ = -1
        self.rel_ = ""
        _ ,self.form_, self.lemma_, self.pos_, self.xpos_, self.morph_, head, self.rel_, _, _ = token_stream.split('\t')
        self.head_ = int( head )


    def print ( self, id, printer ):
        printer ( f"{id}\t{self.form_}\t{self.lemma_}\t{self.pos_}\t{self.xpos_}\t{self.morph_}\t{self.head_}\t{self.rel_}\t_\t_\n" )


class Sentence:


    def __init__ (self, sentence_stream : str ):

        self.tokens_ = []
        for token_stream in sentence_stream.split('\n'):
            self.tokens_.append (Token(token_stream))


    def print ( self, printer ):
        for id, token in enumerate (self.tokens_):
            token.print ( id + 1, printer )
        printer ( '\n' )

    def length ( self ):
        return len ( self.tokens_ )









