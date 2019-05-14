const utils = require('../../misc/utils')
function MakeLike(e) {
    if(e  != '%') {
        return `'%${e}%'`;
    }
    return `'%'`
}

function search(ctx, next) {
    
    var isbn = utils.secure_get(ctx.request.body.isbn, '%')
    var title = utils.secure_get(ctx.request.body.title, '%')
    var publisher = utils.secure_get(ctx.request.body.publisher, '%')
    var description = utils.secure_get(ctx.request.body.description, '%')
    var author = utils.secure_get(ctx.request.body.author, '%')


    var isbn_mand = utils.secure_get(ctx.request.body.isbn_mand, true)
    var title_mand = utils.secure_get(ctx.request.body.title_mand, true)
    var publisher_mand = utils.secure_get(ctx.request.body.publisher_mand, true)
    var description_mand = utils.secure_get(ctx.request.body.description_mand, false)
    console.log(title_mand)
    console.log(ctx.request.body.title_mand)
    isbn_mand = isbn_mand ? 'and' : 'or'
    title_mand = 'and'
    // title_mand = title_mand ? 'and' : 'or'
    publisher_mand = publisher_mand ? 'and' : 'or'

    //isbn_mand = 'and'
   // title_mand = 'and'
    //publisher_mand = 'and'

    isbn = MakeLike(isbn)
    title = MakeLike(title)
    publisher = MakeLike(publisher)
    description = MakeLike(description)
    author = MakeLike(author)

    let failed  = true

    console.log(title_mand)

    console.log(`select distinct ISBN from book where ISBN like ${isbn} ${isbn_mand} book_title like ${title} ${title_mand} publisher like ${publisher} ${publisher_mand} book_desc like ${description}`)
    
    conn.query(`select distinct ISBN from book where ISBN like ${isbn} ${isbn_mand} book_title like ${title} ${title_mand} publisher like ${publisher} ${publisher_mand} book_desc like ${description}`, (err, rows)=>{
        if(err && err.retcode != 0) {
            console.log(err)
            ctx.response.body = {
                ok: false,
                msg: 'SQL Error'
            }
            return
        }
        console.log('search res', rows)
        failed = false
        ctx.session['search'] = {}
        ctx.session['search']['book'] = rows
    })

    if(failed) return
    ctx.response.body = {
        ok: true
    }
}

function fetchSearchResult(ctx, next)
{
    if(ctx.session.search === null || ctx.session.search == undefined) {
        ctx.response.body = {
            ok: false,
            msg: 'No Result Pending'
        }
        return
    }

    ctx.response.body = {
        ok:true,
        payload: ctx.session.search
    }
    ctx.session.search = null
    return
}


module.exports = {
    'POST /api/search': search,
    'POST /api/fetchsearch': fetchSearchResult
}