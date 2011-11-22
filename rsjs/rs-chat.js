var raas_url = "http://localhost:10101";

var RsChat = new (function() {
    var markdown = new Markdown.getSanitizingConverter();

    this.public_chat = function (output, input) {
        var poll_messages = function(el, poll_cb) {
            jQuery.getJSON(raas_url + "/messages/global_chat", function(data) {
                $(data["messages"]).each(function(i, msg){
                    var name = "Unknown";
                    if(msg.from in RsFriends.friend_list) {
                        name = RsFriends.friend_list[msg.from]["name"];
                    }
                   
                    var when = new Date(); when.setTime(msg.send_time * 1000);
                    when = when.toUTCString();

                    var txt = '';
                    try {
                        txt = $(msg.msg).last("p").text();
                        if(!txt) txt = $(msg.msg).text();
                    } finally {
                        if(!txt) txt = msg.msg;
                    }
        
                    var msgel = $('<h1/>').text(name);
                    msgel.append($('<time/>').text(when));
                    msgel = msgel.after(markdown.makeHtml(txt)); 

                    $(output).append($('<section/>').append(msgel));
                });
                if(poll_cb) poll_cb();
            });            
        };

        // make the input form submission async
        input.submit(function(e) {
            e.preventDefault();
            jQuery.ajax({
                type: input.attr("method"),
                url: input.attr("action"),
                data: input.serializeArray(),
                success: function () {
                    $(':text', input).attr('value', '');
                }
            });
        });
        
        // update existing friends data
        RsFriends.fetch_friends();

        // setup timers to poll for incoming messages
        var poll_timer = function(){
                var set_timer = arguments.callee;
                setTimeout(function(){poll_messages(output, set_timer);}, 1000);
        };
        poll_messages(output, poll_timer);
    };
});

var RsFriends = new (function() {
    this.friend_list = {};
    this.fetch_friends = function() {
        jQuery.getJSON(raas_url + "/friends", function(data) {
            RsFriends.friend_list = data;
            return data;
        });
    };
})();


