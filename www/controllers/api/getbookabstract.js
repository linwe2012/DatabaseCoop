'use strict'
function getAbstractByISBN(ctx, next) {
    let isbn = ctx.request.body.isbn_start || '0'
    let cnt = ctx.request.body.cnt || 10
    let request_copy_info = ctx.request.body.request_copy_info || true
    let res = []
    conn.query(`select * from book where ISBN >= '${isbn}' order by ISBN asc limit ${cnt}`, function (err, books){
        if(err && err.retcode != 0) { console.log(err)}
        for(let b of books) {
            var book = {
                ISBN: b.ISBN,
                title: b.book_title,
                num_pages: b.num_pages,
                img_url: b.cover_img,
                description: b.book_desc,
                format: b.edition_format,
                publisher: b.publisher,
                date_published: b.date_published,
                authors: [],
                in_copies: [],
                out_copies: [],
                not_copies: [],
                language_code: b.book_lang,
                price: b.price,
            }
            conn.query(`select author_id from written_by where ISBN='${b.ISBN}'`, (err, authors)=>{
                if(err && err.retcode != 0) { console.log(err)}
                for(let au of authors) {
                    conn.query(`select * from author where author_id=${au.author_id}`, (err, author)=>{
                        if(err && err.retcode != 0) { console.log(err)}
                        if(author.length) {
                            book.authors.push(author[0])
                        }
                    })
                }
            }) 
            
            if(request_copy_info) // this if controls for loop below
            conn.query(`select * from copy where ISBN = '${b.ISBN}'`, (err, copies)=>{
                if(err && err.retcode != 0) { console.log(err)}
                for(let copy of copies) {
                    if(copy.stat === 'in') {
                        book.in_copies.push(copy)
                    }
                    else if(copy.stat === 'out') {
                        conn.query(`select return_time from borrow where ISBN='${b.ISBN}' and copy_id=${copy.copy_id}`, (err, rec)=>{
                            if(err && err.retcode != 0) { console.log(err)}
                            if(rec.length) {
                                copy['return_time'] = rec[0].return_time
                            }
                        })
                        book.out_copies.push(copy)
                    }
                    else /* stat = 'not' */{
                        book.not_copies.push(copy)
                    }
                }
            })
            res.push(book)
        }
    })
    // console.log(res)
    ctx.response.type = 'json'
    ctx.response.body = res;
}


module.exports = {
    'POST /api/getbookabstract_by_isbn': getAbstractByISBN
}