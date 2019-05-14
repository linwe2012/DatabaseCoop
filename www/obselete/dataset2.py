"""
from goodreads import client
import json
gc = client.GoodreadsClient('DxSHlcbnweXsSqfuGMj2eQ', 'UmL7puj9kQspCyB9oG0DPvG4DbkyucnWZ4TGd8CLjI')

book = gc.book(1)
authors = book.authors
print(book)
print(authors[0].name)
print(book.small_image_url)
print(book.description)
print(book.isbn13)
print(book.original_publication_year)
print(book.language_code)
print(authors[0].id)*/
"""
import traceback
import goodreads_api_client as gr
import json
import time

client = gr.Client(developer_key='DxSHlcbnweXsSqfuGMj2eQ')
i = 21
o = open('./dataset.txt', 'a+')
while i < 100:
    begin = time.localtime(time.time())
    err_cnt = 1
    while 1:
        try:
            book = client.Book.show(str(i))
            keys_wanted = ['id', 'title', 'isbn13', 'image_url', 
            'publication_year', 'publication_month', 'publication_day',
            'publisher', 'language_code', 'description', 'num_pages', 'format', 'authors']
            mbook = {k:v for k,v in book.items() if k in keys_wanted}
            if i == 1:
                print(json.dumps(mbook))
            break
        except:
            print('\r'+str(i) +': ' + '(' + str(err_cnt) + ')Network is flaky, Retry')
            traceback.print_exc()
            err_cnt += 1
            if err_cnt > 20:
                print('Too many failures, Abort')
            break
    print('\r' + str(i) + ': done')
    o.write(json.dumps(mbook)+'\n')
    end = time.localtime(time.time())
    interval = (end.tm_min - begin.tm_min) + end.tm_sec - begin.tm_sec - 1
    if interval > 0:
        time.sleep(interval)
    i = i+1

print(book)