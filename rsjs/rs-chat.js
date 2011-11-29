var raas_url = "http://localhost:10101";

var RsChat = new (function() {
    var markdown = new Markdown.getSanitizingConverter();

    this.public_chat = function (output, input) {
        var poll_messages = function(el, poll_cb) {
            jQuery.getJSON(raas_url + "/messages/global_chat", function(data) {
                $(data["messages"]).each(function(i, msg){
                    var name = "Unknown";
                    if(RsFriends.is_known_id(msg.from)) {
                        name = RsFriends.from_id(msg.from)["name"];
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
                    $(':text, textarea', input).attr('value', '');
                }
            });
        });
       
        // also attach a keyup handler so we can submit via text-area
        input.first("textarea").keyup(function(e) {
            e = e || event;
            if (e.keyCode === 13 && e.ctrlKey) {
                input.submit();
            }
            return true;
        });


        // setup timers to poll for incoming messages
        var poll_timer = function(){
                var set_timer = arguments.callee;
                setTimeout(function(){poll_messages(output, set_timer);}, 1000);
        };
        poll_messages(output, poll_timer);

        $(':text, textarea', input).focus();
    };
});

var RsFriends = new (function() {
    this._friend_list = {};
    this.fetch_friends = function() {
        jQuery.getJSON(raas_url + "/friends", function(data) {
            var do_trigger = false;
            if(data != RsFriends._friend_list) do_trigger = true;
            var old_friends = RsFriends._friend_list;
            RsFriends._friend_list = data;
            if(do_trigger) $(RsFriends).trigger('RsFriends_list_updated', [old_friends, data]);
            return data;
        });
    };

    this.is_known_id = function(id) {
        return id in RsFriends._friend_list;
    };

    this.from_id = function(id) {
        return RsFriends._friend_list[id];
    };

    // friend list view, it dumps new html elements into whatever el is
    // whenever the friend list changes.
    this.friend_list = function(el) {
        $(RsFriends).bind('RsFriends_list_updated', function(ev, oldlist, newlist) {
            jQuery.each(newlist, function (index, value) {
                el.append($('<li/>').text(value["name"]));
            });
        });
    };
})();


