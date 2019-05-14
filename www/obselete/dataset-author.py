import traceback
import goodreads_api_client as gr
import json
import time
import os

client = gr.Client(developer_key='DxSHlcbnweXsSqfuGMj2eQ')

book_files= open('./dataset.txt', 'r')
author_files = open('./dataset-author.txt', 'a')
known = set()
for line in book_files.readlines():
    print(line)
    obj = json.loads(line)
    # print(obj['authors']['author'])
    for author in obj['authors']['author']:
        if  int(author['id']) in known:
            continue
        known.add(int(author['id']))
        au = gr.Author.show(author['id'])
        keys_wanted = ['name', 'image_url', 'about', 'gender', 'born_at', 'died_at', 'hometown']
        mau = {k:v for k,v in au.items() if k in keys_wanted}
        author_files.write(json.dumps(mau)+'\n')
        
