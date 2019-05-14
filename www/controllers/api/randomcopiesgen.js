'use strict'

const UserPriv = require('./internal/userprivvalidate')
function gen(ctx, next) {
    if(!UserPriv.isAdmin(ctx, next)) return
    let avg = ctx.request.body.avg || 5;
    var insert_branch = false
    conn.query('select * from branch', function(err, rows){
        if(rows.length == 0) insert_branch = true
        // console.log('hi')
    })
    if(insert_branch) {
        // console.log('huuu')
        conn.query(`insert into branch values (0, 'SpaceLib', 'Some nebula in space')`,(err)=>{if(err.retcode !=0){console.log(err)}})
        conn.query(`insert into branch values (1, 'SeaLib', '#10, Cameron Street, Mariana Trench')`,(err)=>{if(err.retcode !=0){console.log(err)}})
        conn.query(`insert into branch values (2, 'Shining And Then Bang', '#8, East End, SuperNova M13, Andromeda Galaxy')`,(err)=>{if(err.retcode !=0){console.log(err)}})
        conn.query(`insert into branch values (3, 'Freeze', 'South Pole, Antarctic, Earth')`,(err)=>{if(err.retcode !=0){console.log(err)}})
    }

    var branches = [];
    conn.query(`select branch_id from branch`, function(err, rows){
        branches = rows
    })

    conn.query('select ISBN from book', function(err, books) {
        for(let book of books) {
            
            var num = Math.round(Math.random() * avg + avg / 2);
            for(let cpy_id = 0; cpy_id < num; ++cpy_id){
                let branch = branches[Math.round(Math.random() * branches.length) % branches.length]
                conn.query(`insert into copy values ('${book.ISBN}', ${cpy_id}, ${branch.branch_id}, 1, 'in')`, function(err){
                    if(err.retcode) {console.log(err); return}
                })
            }
        }
    })

    ctx.response.body = {
        ok: true
    }
}


module.exports = {
    'POST /api/randomcopiesgen': gen
}