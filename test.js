mysql = require('./Debug/libmysql.node')

conn = mysql.createConnection({
  database: 'library',
  user: 'lib',
  password: 'lib'
})

console.log(conn.constructor.name)
retcode = conn.connect()



hanlde = conn.prepare(`insert into book 
(ISBN, book_title, publisher, date_published, num_pages, cover_img, book_desc, edition_format) 
values (?, ?, ?, ?, ?, ?, ?, ?)`)

author_by_id= conn.prepare(`select * from author where author_id=?`)
insert_author= conn.prepare(`insert into author (author_id, author_name, author_img_url) values (?,?,?)`)
insert_written_by= conn.prepare(`insert into written_by (ISBN, author_id) values(?,?)`)
conn.query(`insert into book (ISBN) values( '1234567890123')`, function(err){})
authors = [{id:1, name: '100', img: 'uuu'}, {id:4, name: '100', img: 'uuu'}, {id:2, name: '2132', img: 'uuu'}, {id:2, name: '1232', img: 'uuu'}]
for(author of authors) {
  conn.query(author_by_id, [author.id], function(err, rows) {
      
      if(err.retcode != 0){ 
          console.log([author.id])
          console.log(err); failed = 1
      }

      console.log('exits row:', rows)
      if(rows.length == 0) {
        console.log('new user')
        conn.query(insert_author, [author.id, author.name, null], function(err, rows) {
          if(err.retcode != 0) { 
              console.log([author.id, author.name, null])
              console.log(err); failed = 1
          }
        })
        console.log('end new user')
      }
      
  }) 
  console.log('new user')
  conn.query(insert_written_by, ['1234567890123', author.id], function(err, rows) {
      if(err.retcode != 0) { console.log(err); failed = 1}
  })
}

//var name = 'uuu';
//conn.query(hanlde, [name, name], function(err, rows){console.log(err)})
//conn.query('show table', function(err, rows){console.log(err)})
// This is for c internal debuging
conn.query(hanlde, 
  ['hi', 'uuu', 'b.publisher', {date:10, year: 2020, month: 11}, 100, 'b.image_url', 'b.description', 'b.format'],
  function(err, rows, fields){
      if(err.retcode != 0) {
          console.log(err)
          failed = 1
      }
})

conn.c_internal_test();
sqltypes = mysql.fetchTypes();
sqltypesName = mysql.fetchTypeNameArray()

console.log(sqltypes, sqltypesName)

conn.query('select * from book', (err, rows, fields)=>{
  console.log(err)
  console.log(rows)
  console.log(fields)
})


conn.query('select * from meta', function(err, rows, fields){
  console.log(err)
  console.log(rows)
  console.log(fields)
})

conn.query('select * from book', function(err, rows, fields){
  console.log(err)
  console.log(rows)
  console.log(fields)
})

conn.query('show tables', function(err, rows, fields){
  if(err != 0) return;
  for(row of rows){
    console.log(row[fields[0]])
  }
})












