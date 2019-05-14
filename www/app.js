mysql = require('../Debug/libmysql.node')

conn = mysql.createConnection({
    database: 'library',
    user: 'lib',
    password: 'lib'
})

httpstatus = {
    unauthorized: 401,
    forbidden: 403
}
  
var retcode = conn.connect()
sqltypes = mysql.fetchTypes()

sqlhandles = {
    login: conn.prepare('select * from usr where usr_name=?'),

    new_user: conn.prepare(`insert into usr 
    (usr_name, usr_passwd, time_registered, max_num_book_allowed, max_renew_allowed) 
    values (?,?, CURRENT_TIMESTAMP(), -1, -1)`),
    
    batch_insert_book: conn.prepare(`insert into book 
    (ISBN, book_title, publisher, date_published, num_pages, cover_img, book_desc, edition_format, book_lang) 
    values (?, ?, ?, ?, ?, ?, ?, ?, ?)`),

    author_by_id: conn.prepare(`select * from author where author_id=?`),
    insert_author: conn.prepare(`insert into author (author_id, author_name, author_img_url) values (?,?,?)`),
    insert_written_by: conn.prepare(`insert into written_by (ISBN, author_id) values(?,?)`),
    insert_borrow: conn.prepare(`insert into borrow values (?, ?, ?, ?, ?, ?)`)
}

config = {
    borrow_time : {
        year: 0,
        month: 2,
        date: 0,
        hour: 0,
        minute: 0,
        second: 0,
        fraction: 0
    },

    renew_time:  {
        year: 0,
        month: 2,
        date: 0,
        hour: 0,
        minute: 0,
        second: 0,
        fraction: 0
    },

    min_time_gap_renew: {
        year: 0,
        month: 1,
        date: 0,
        hour: 0,
        minute: 0,
        second: 0,
        fraction: 0
    },

    returnbook_warning: {
        year: 0,
        month: 1,
        date: 0,
        hour: 0,
        minute: 0,
        second: 0,
        fraction: 0
    }
}

utils = require('./misc/utils')

for (let name of Object.keys(sqlhandles)) {
    handle = sqlhandles[name]
    if(handle.err) {
        console.log('Unable to init handle:', name)
        console.log(handle.err)
        process.exit()
    }
}

if(retcode != 0) {
    console.log('Unable to connect to sql')
}

const Koa = require('koa');

const session=require('koa-session');

const bodyParser = require('koa-bodyparser');

const controller = require('./controller');

const templating = require('./templating');

const app = new Koa();

const isProduction = false;

// log request URL:
app.use(async (ctx, next) => {
    console.log(`Process ${ctx.request.method} ${ctx.request.url}...`);
    var
        start = new Date().getTime(),
        execTime;
    await next();
    execTime = new Date().getTime() - start;
    ctx.response.set('X-Response-Time', `${execTime}ms`);
});

// static file support:
if (! isProduction) {
    let staticFiles = require('./static-files');
    app.use(staticFiles('/static/', __dirname + '/static'));
}
app.keys = ['this is my secret'];

app.use(session({
    key: 'koa:session', 
    maxAge: 86400000, /** (number) maxAge in ms (default is 1 days)，cookie的过期时间，这里表示2个小时 */
    overwrite: true, /** (boolean) can overwrite or not (default true) */
    httpOnly: true, /** (boolean) httpOnly or not (default true) */
    signed: true, /** (boolean) signed or not (default true) */
  },app));


// parse request body:
app.use(bodyParser({
    formLimit: '5mb'
}));

// add nunjucks as view:
app.use(templating('views', {
    noCache: !isProduction,
    watch: !isProduction
}));

// add controller:
app.use(controller(['controllers', 'controllers/api']));

app.listen(3000);
console.log('app started at port 3000...');