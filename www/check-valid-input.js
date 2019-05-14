'use strict'

function username(str) {
        return /^[a-zA-Z_.]{3,255}$/.test(str)
    }

function password(str) {
    return /^[a-zA-Z0-9_.]{3,255}$/.test(str)
}


module.exports = {
    username: username,
    password: password
}