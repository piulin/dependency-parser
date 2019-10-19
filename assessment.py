import set

def uas (gt : set, predicted : set):

    uas,pred,total_tokens = assess( gt, predicted, lambda ltok,ptok: int ( ltok.head_ == ptok.head_ ) )
    return {"uas":uas ,'successfully_predicted': pred, 'tokens':total_tokens}

def las (gt : set, predicted : set):

    las,pred,total_tokens = assess( gt, predicted, lambda ltok,ptok: int ( ltok.head_ == ptok.head_ and ltok.rel_ == ptok.rel_ ) )
    return {"las":las ,'successfully_predicted': pred, 'tokens':total_tokens}

def assess (gt : set, predicted : set, func):

    fulfilled = 0
    total_tokens = gt.count_tokens()

    for lsen,psen in zip(gt.sentences_, predicted.sentences_):
        for ltok,ptok in zip ( lsen.tokens_, psen.tokens_ ):
            fulfilled += func (ltok,ptok)

    return {fulfilled*100/total_tokens , fulfilled, total_tokens}
