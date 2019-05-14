const UserPriv = require('./internal/userprivvalidate')
function secureInt(i, fallback) 
{
    i = parseInt(i)
    if(isNaN(i)) return fallback;
    return i;
}

function insertAuthor(isbn, author) {
    var failed = 0
    author.id = secureInt(author.id, 0)
    conn.query(sqlhandles.author_by_id, [author.id], function(err, rows) {
        if(err.retcode != 0){ 
            console.log([author.id])
            console.log(err); failed = 1
        }
        if(rows.length != 0) return failed;
        conn.query(sqlhandles.insert_author, [author.id, author.name, author.image_url['#text']], function(err, rows) {
            if(err.retcode != 0) { 
                console.log([author.id, author.name, author.image_url['#text']])
                console.log(err); failed = 1
            }
        })
    }) 
    conn.query(sqlhandles.insert_written_by, [isbn, author.id], function(err, rows) {
        if(err.retcode != 0) { console.log(err); failed = 1}
    })
    return failed
}
function loadfromgoodread(ctx, next) {
    if(!UserPriv.isAdmin(ctx, next)) return
    let jsons = ctx.request.rawBody;
   // console.log(typeof jsons)
    var failed = 0
    for (let json of jsons.split(/[\r\n]+/)) {
        if(json.length < 2)  {
            continue
        }
        let = b = JSON.parse(json)
        if(!b.isbn13) {
            continue
        }
        let pubdate = {
            year: secureInt(b.publication_year, 999),
            month: secureInt(b.publication_month, 1),
            date: secureInt(b.publication_day, 1)
        }
        b.num_pages = secureInt(b.num_pages, 0)
        conn.query(sqlhandles.batch_insert_book, 
            [b.isbn13, b.title, b.publisher, pubdate, b.num_pages, b.image_url, b.description, b.format, b.language_code],
            function(err, rows, fields){
                if(err.retcode != 0) {
                    console.log('Insert Book Error:', err)
                }
                if(Array.isArray(b.authors.author)){
                    for(author of b.authors.author) {
                        if(insertAuthor(b.isbn13, author))
                            return
                        
                    }
                }else {
                    if(insertAuthor(b.isbn13, b.authors.author))
                    return
                }
        })
        
       // if(failed){
      //      return
        //}
    }
    ctx.response.type = 'json'
    ctx.response.body = {
        ok: true
    }
    
    // ctx.response.body = csv
}


module.exports = {
    'POST /api/loadfromgoodread':loadfromgoodread
}