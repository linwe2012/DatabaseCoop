
module.exports = {
 isAdmin: (ctx, next) =>{
     if(ctx.session.max_num_book_allowed < 0 && ctx.session.max_renew_allowed < 0) {
         return true
     }
    return false
},

loggedIn: (ctx, next) =>{
    if(ctx.session.usr_name === undefined || ctx.session.usr_name === null) return false
    return true
}
}