class Token:

    def __init__ (self, id=None, form=None, lemma=None, pos=None, xpos=None, morph=None, head=None, rel=None, token_stream=None ):
        if token_stream != None:
            id ,self.form_, self.lemma_, self.pos_, self.xpos_, self.morph_, head, self.rel_, _, _ = token_stream.split('\t')
            self.head_ = int( head )
            self.id_ = int( id )
        else:
            self.id_ = id
            self.form_ = form
            self.lemma_ = lemma
            self.pos_ = pos
            self.xpos_ = xpos
            self.morph_ = morph
            self.head_ = head
            self.rel_ = rel

    def print ( self, printer ):
        printer ( f"{self.id_}\t{self.form_}\t{self.lemma_}\t{self.pos_}\t{self.xpos_}\t{self.morph_}\t{self.head_}\t{self.rel_}\t_\t_\n" )


class Sentence:


    def __init__ (self, tokens = None, sentence_stream=None ):
        if sentence_stream != None:
            self.tokens_ = [ Token(0,'Root','root','_','_','_',-1,'_') ]
            for token_stream in sentence_stream.split('\n'):
                self.tokens_.append (Token(token_stream=token_stream))
        else:
            self.tokens_ = tokens


    def print ( self, printer ):
        for id, token in enumerate ( self.tokens_ [ 1: ] ):
            token.print ( printer )
        printer ( '\n' )

    def length ( self ):
        return len ( self.tokens_ [ 1: ] )









