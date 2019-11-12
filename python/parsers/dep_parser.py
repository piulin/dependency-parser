class State:

    def __init__(self, buff, stack, arcs, heads ):

        self.buff_ =  buff
        self.stack_ = stack
        self.arcs_ = arcs
        self.heads_ = heads

    def __str__(self):
        return f""" Buffer state: {humanize (self.buff_)}
 Stack state: { humanize (self.stack_) } 
 Arcs state: { humanize_arcs(self.arcs_) }
 Heads state: { humanize_heads(self.heads_)}
                """

def humanize (stack_or_buff):
    return [ tk.form_ for tk in stack_or_buff ]

def humanize_arcs (arcs):
    return { k.form_: humanize(v) for k,v in arcs.items() }

def humanize_heads (heads):
    return { k.form_:v.form_ if v != None else None for k,v in heads.items() }

