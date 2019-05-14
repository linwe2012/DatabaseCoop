function isNotNullOrUndef(a) {
    if(a === null || a === undefined) return false;
    return true
}

module.exports = {
    add_timestamp: (l, r)=>{
        let carry = 0
        let res = {}
        res['fraction'] = (l.fraction || 0) + (r.fraction || 0)
        res['second'] = (l.second || 0) + (r.second || 0)
        if(res['second'] > 59 ) {
            res['second'] -= 60;
            carry = 1;
        }
        else if(res['second'] < 0) {
            res['second'] += 60
            carry = -1;
        }
        else carry = 0

        res['minute'] = (l.minute || 0) + (r.minute || 0) + carry
        if(res['minute'] > 59 ) {
            res['minute'] -= 60;
            carry = 1;
        }
        else if(res['minute'] < 0 ) {
            res['minute'] += 60;
            carry = -1;
        }
        else carry = 0

        res['hour'] = (l.hour || 0) + (r.hour || 0) + carry
        if(res['hour'] > 23 ) {
            res['hour'] -= 24;
            carry = 1;
        }
        else if(res['hour'] < 0 ) {
            res['hour'] += 24;
            carry = -1;
        }
        else carry = 0

        res['date'] = (l.date || 0) + (r.date || 0) + carry
        if(res['date'] > 28 ) {
            res['date'] -= 28;
            carry = 1;
        }
        if(res['date'] < 1 ) {
            res['date'] += 28;
            carry = -1;
        }
        else carry = 0

        res['month'] = (l.month || 0) + (r.month || 0) + carry
        if(res['month'] > 12 ) {
            res['month'] -= 12;
            carry = 1;
        }
        else if(res['month'] < 1 ) {
            res['month'] += 12;
            carry = -1;
        }
        else carry = 0

        res['year'] = (l.year || 0) + (r.year || 0) + carry

        return res
    },
/*
    isNotNullOrUndef: (a)=>{
        if(a === null || a === undefined) return false;
        return true
    },*/
    sub_time(l, rhs) {
        let r = {};
        Object.assign(r, rhs)
        for(let k of Object.keys(r)) {
            r[k] = - r[k]
        }

        return this.add_timestamp(l, r)
    },

    
    time_cmp: (l, r)=>{
        // Now i know how much i love macros & c++
        if(isNotNullOrUndef(l.year) && isNotNullOrUndef(r.year) && l.year < r.year) return -1;
        else if(isNotNullOrUndef(l.year) && isNotNullOrUndef(r.year) && l.year > r.year)  return 1;

        if(isNotNullOrUndef(l.month) && isNotNullOrUndef(r.month) && l.month < r.month) return -1;
        else if(isNotNullOrUndef(l.month) && isNotNullOrUndef(r.month) && l.month > r.month) return 1;

        if(isNotNullOrUndef(l.date) && isNotNullOrUndef(r.date) && l.date < r.date) return -1;
        else if(isNotNullOrUndef(l.date) && isNotNullOrUndef(r.date) && l.date > r.date) return 1;

        if(isNotNullOrUndef(l.hour) && isNotNullOrUndef(r.hour) && l.hour < r.hour) return -1;
        else if(isNotNullOrUndef(l.hour) && isNotNullOrUndef(r.hour) && l.hour > r.hour) return 1;

        if(isNotNullOrUndef(l.minute) && isNotNullOrUndef(r.minute) && l.minute < r.minute) return -1;
        else if(isNotNullOrUndef(l.minute) && isNotNullOrUndef(r.minute) && l.minute > r.minute) return 1;

        if(isNotNullOrUndef(l.second) && isNotNullOrUndef(r.second) && l.second < r.second) return -1;
        else if(isNotNullOrUndef(l.second) && isNotNullOrUndef(r.second) && l.second > r.second) return 1;

        return 0;
    },

    get_timestamp: ()=>{
        let d = new Date()
        return {
            year: d.getFullYear(),
            month: d.getMonth() + 1,
            date: d.getDate(),
            hour: d.getHours(),
            minute: d.getMinutes(),
            second: d.getSeconds(),
            fraction: 0
        }
    },

    time_to_str: (t)=>{
        return `${t.year}-${t.month}-${t.date} ${t.hour}:${t.minute}:${t.second}`
    },

    secure_get: (e, fallback)=>{
        if(e === null || e === undefined) return fallback
        return e
    }
}