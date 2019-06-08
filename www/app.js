mysql = require('../Debug/libmysql.node')


conn = mysql.createConnection({
    database: 'whatever',
    user: 'i dont care',
    password: 'doesnt matter'
})

httpstatus = {
    unauthorized: 401,
    forbidden: 403
}
  
var retcode = conn.connect()

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