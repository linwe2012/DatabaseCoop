// inspired by https://gist.github.com/hitswa/b0d5c161a0507a54d2e4e72b12dd4537
function goodReadById(id) {

    var key = "DxSHlcbnweXsSqfuGMj2eQ"; // replace with your key

    var url = "https://www.goodreads.com/book/show.xml/?" + "?key=" + key + '&id=' + id;

    var xhr = new XMLHttpRequest();
    xhr.open('GET', "http://query.yahooapis.com/v1/public/yql" + "?q=select * from xml where url=\"" + key + "\"")
    xhr.onreadystatechange = function() {
        console.log(xhr.responseText)

    }/*
    $.get("http://query.yahooapis.com/v1/public/yql",
        {
            q: "select * from xml where url=\""+url+"\"",
            format: "json"
        },
        function(json){

            if(json.query.results.error === "Page not found") {
                console.log("no book found");
            } else {

                var book = json.query.results.GoodreadsResponse.book;

                var title           = book.title;
                var isbn10          = book.isbn;
                var isbn13          = book.isbn13;
                var country_code    = book.country_code;
                var image_url       = book.image_url;
                var small_image_url = book.small_image_url;
                var publisher       = book.publisher;
                var description     = book.description;

                var all_authors = book.authors.author;


                
                if(book.authors.author.length===undefined) { //single author

                    var authors = [all_authors.name];

                } else { // multiple authors

                    var author_count = all_authors.length

                    var authors = [];

                    for( i=0 ; i<author_count ; i++) {
                        authors.push(all_authors[i].name)
                    }

                }

                var book_object = {
                        "title"           : title,
                        "isbn10"          : isbn10,
                        "isbn13"          : isbn13,
                        "country_code"    : country_code,
                        "image_url"       : image_url,
                        "small_image_url" : small_image_url,
                        "publisher"       : publisher,
                        "description"     : description,
                        "authors"         : authors,
                      }
                console.log(book_object);

            }
        }
    );*/
}
