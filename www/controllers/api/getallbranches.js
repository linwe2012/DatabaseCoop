function getAllBranches(ctx, next)
{
    conn.query('select * from branch', (err, rows)=>{
        if(err && err.retcode != 0) {
            console.log(err);
            ctx.response.body = {ok: false, msg: 'Unable to fetch data'}
            return
        }

        ctx.response.body = {
            ok:true,
            payload: rows
        }
    })
}

module.exports = {
    'POST /api/getallbranches': getAllBranches
}