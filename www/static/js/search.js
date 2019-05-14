async function GlobalSearch()
{
    let val = $id('search-ipt').value
    let res = await $post({
        url: global.baseurl + '/api/search',
        params: `title=${val}`
    })
    res = JSON.parse(res)
    if(res.ok)
        window.location = '/list'
    else{
        console.log(res)
    }
}