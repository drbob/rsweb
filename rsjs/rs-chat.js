/*
 * adds a chat line to the output box
 *
  });
if(poll_cb) poll_cb();

**/

Date.prototype.toISO8601 = function () {
    d = this;
    function pad(n){
        return n < 10 ? '0'+n : n
    }
    return d.getUTCFullYear()+'-'
    + pad(d.getUTCMonth()+1)+'-'
    + pad(d.getUTCDate())+'T'
    + pad(d.getUTCHours())+':'
    + pad(d.getUTCMinutes())+':'
    + pad(d.getUTCSeconds())+'Z'
}

var RS = {
// FIXME: deduce this from window.location?
root_url: "http://localhost:10101"
};

RS.Chat = {
    _poll_interval: 1000,

    room: function(settings) {
        settings.url = '/messages/im/' + settings.id;
        return settings;
    },

    _fetch_chat_queues: function (queue_cb, complete_cb) {
        var room_url = RS.root_url + '/messages/im'; 

        jQuery.getJSON(room_url, function(data) {
            if(queue_cb) jQuery.each(data, function(room_id, queue) {
                if(queue.length) queue_cb(RS.Chat.room({id: room_id}), queue);
            });
            if(complete_cb) complete_cb();
        });
    },

    send_message: function(options) {
        var real_options = jQuery.extend(
            {
             url: options.room,
             type: "POST"
            },
        options);
        return jQuery.ajax(real_options);
    },

    monitor_chat_queues: function(queue_cb) {
        RS.Chat._fetch_chat_queues(queue_cb,
            (function() {
                var loop = arguments.callee;
                setTimeout(
                    function() {
                        RS.Chat._fetch_chat_queues(queue_cb, loop);
                    },
                RS.Chat._poll_interval); 
            }));
    }
};


RS.Identities = {
    _ident_list: {},
    _poll_interval: 1000,

    _fetch_idents: function(update_cb, complete_cb) {
        jQuery.getJSON(RS.root_url + "/identities", function(data) {
            var do_trigger = false;
            if(data != RS.Identities._ident_list) do_trigger = true;
            var old_friends = RS.Identities._ident_list;
            RS.Identities._ident_list = data;
            if(do_trigger && update_cb) update_cb(old_friends, data);
            if(complete_cb) complete_cb(old_friends, data);
        });
    },

    monitor_known_idents: function (ident_update_cb) {
        RS.Identities._fetch_idents(ident_update_cb,
            (function() {
                var loop = arguments.callee;
                setTimeout(
                    function() {
                        RS.Identities._fetch_idents(ident_update_cb, loop);
                    },
                RS.Identities._poll_interval); 
            })
        );
    },

    is_known_id: function(id) {
        return id in RS.Identities._ident_list;
    },

    from_id: function(id) {
        return jQuery.extend({id: id}, RS.Identities._ident_list[id]);
    },
};

RS.UI = {
    Markdown: (new Markdown.Converter()), 

    /* Attaches event handlers and other UI things needed
     * to make a chat input area out of a form.
     */
    chat_input: function(input) {
        input.addClass('rs-chat-input');
        input.tabs();

        input.submit(function(e){ RS.Chat.send_message({
            room: input.attr("action"),

            data: input.serializeArray(),
        
            beforeSend: function () {
                $(':submit', input).attr('disabled', 'disabled');
                $(':text, textarea', input).attr('disabled', 'disabled')
            },

            success: function () {
                $(':text, textarea', input).attr('value', '');
            },

            complete: function() {
                $(':text, textarea', input).removeAttr('disabled');
                $(':submit', input).removeAttr('disabled');
            }
        }); e.preventDefault(); });

        // also attach a keyup handler so we can submit via text-area
        input.first("textarea").keyup(function(e) {
            e = e || event;
            if (e.keyCode === 13 && e.ctrlKey) {
                input.submit();
            }
            return true;
        });
    },

    tabbed_chat_output: function (output) {
        // enable the jQuery Tabs
        output.tabs();

        output.addClass('rs-chat-output');

        // listen for chat events
        output.addClass('__rs-chat-incoming');
        output.bind('RS.Chat.incoming', function (ev, room, queue) {
            var tabid = '#rs-chat_' + room.id;
            var tab = $(tabid);
            var tabscreate = false;
            if(!tab.length) {
                output.tabs('add', tabid, room.id);
                tab = $(tabid);
                tabscreate = true;
            }
            
            tab.data('RS.Chat.room', room);
            
            jQuery.each(queue, function(i, msg){
                RS.UI.append_chat_message(tab, msg);
            }); 
            
            // synthesise a tabselect since
            // tabshow triggers too early for the first tab that is created
            // and this method means that the room data will be populated
            if(tabscreate) output.trigger('tabsselect', {panel: tab});
        });

        output.bind('tabsselect', function(ev, tab) {
            // when showing a tab, rebind the input forms
            // destination URL so that the message is sent to the right place
            var room = $(tab.panel).data('RS.Chat.room');
            $(".rs-chat-input").attr('action', room.url);
        });

        // synthesise a global message to open that chat room
        output.trigger('RS.Chat.incoming', [RS.Chat.room({id: 'public'}), []])
    },

    append_chat_message: function (tab, msg) {
        var msghtml = msghtml = $($("<div>").html(msg.msg));
        var txt = msghtml.find('p').text();
        if(!txt) txt = msghtml.text(); 
        
        var who = RS.UI.ident_label($("<a/>"), RS.Identities.from_id(msg.from));
        var sent = new Date(); sent.setTime(msg.send_time * 1000);
        var when = $('<time/>').attr('datetime', sent.toISO8601())
                               .text(sent.toISO8601())
                               .timeago();
        
        txt = RS.UI.Markdown.makeHtml(txt); 
        var dt = $('<dt/>');
        var dd = $('<dd/>');
        
        var last_sender = tab.find('dt > .rs-ident-label').last();
        var msgclass = 'change-speaker';
        // msg is from same as the last person
        if(last_sender.length && last_sender.hasClass('rs-ssl-id_' + msg.from)) {
            msgclass = 'same-speaker';
        }
        
        dt.append(who).append(when).addClass(msgclass); 
        dd.append(txt);
        dt.appendTo(tab);
        dd.appendTo(tab);
    },

    /* Send out the new queue data to whatever elements care about it. 
     */
    trigger_chat_incoming: function(room, queue) {
        $('.__rs-chat-incoming').trigger('RS.Chat.incoming', [room, queue]);
    },

    // IDENT STUFF /////////////////////////////////////////

    ident_label: function(el, ident) {
        if(!el.hasClass('rs-ident-label')) {
            ident = jQuery.extend({name: 'Unknown',
                connect_state: 0,
                gpg_id: 'None',
            }, ident);

            if(ident != undefined && ident.name != undefined) {
                el.text(ident.name);
            } else {
                el.text("Unknown");
            }

            if(ident['connect_state'] & 0x4) {
                el.removeClass('offline');
                el.addClass('online');
            } else {
                el.removeClass('online');
                el.addClass('offline');
            }

            el.addClass('rs-ssl-id_' + ident['id']);
            el.addClass('rs-ident-label');

            el.attr('title', ident['gpg_id'] + '/' + ident['id']);

            el.draggable({
                revert: 'invalid',
            });
            el.parent().draggable({
                connectToSortable: '#friend_list'
            });
            // add a click handler that will pop up
            // a panel to allow editing of trust relationships
            // with people you already know
            console.log('click');console.log(el);
            el.click(RS.UI.ident_edit_panel);
        }

        el.data('rs-identity', ident);
        return el; 
    },

    ident_edit_panel: function() {
    console.log(this);
        self = $(this);
        var pop = self.parent().find('.pop-out-panel');
        if(pop.length == 0) {
            pop = RS.UI.template('#ident_edit_panel_template').hide().insertAfter(self);
        } 
        pop.toggle(200);
    },


    template: function(selector) {
        var tpl = $(selector).clone();
        tpl.show();
        return tpl;
    },

    trigger_ident_change: function(oldlist, newlist) {
        $('.__rs-identities-update').trigger('RS.Identities.update', [oldlist, newlist]);
        
        // automatically update existing ident labels
        // when their content actually changes
        jQuery.each(newlist, function (index, ident) {
            // FIXME: make a proper equals() operator for idents
            if(!(index in oldlist) || oldlist[index]['connect_state'] != ident['connect_state'])  {
                console.log([index, index in oldlist, oldlist[index], newlist[index]]);
                var existing = $('.rs-ssl-id_' + index);
                existing.each(function(i){ RS.UI.ident_label($(this), ident);});
            }
        });
    },

    /* Configures a friend list that will respond to identity updates
    */
    friend_list: function (elem) { 
        elem = $(elem);
        elem.sortable();
        elem.addClass('__rs-identities-update');
        elem.bind('RS.Identities.update', function(ev, oldlist, newlist) {
            jQuery.each(newlist, function(i, ident) {
                // skip ones that arent new
                if(i in oldlist) return;
                RS.UI.friend_list_ident(elem, ident);
            });
        });

        return elem;
    },

    friend_list_ident: function (elem, ident) {
        var anch = RS.UI.ident_label($("<a>"), ident);
        anch.attr('href', '#');
        return $('<li/>').append(anch).appendTo(elem);
    },
};


