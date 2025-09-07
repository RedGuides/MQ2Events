---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2events.124/"
support_link: "https://www.redguides.com/community/threads/mq2events.66828/"
repository: "https://github.com/RedGuides/MQ2Events"
config: "MQ2Events_CharacterName.ini, MQ2Events.ini"
authors: "Bardomatic, Sic, brainiac, eqmule"
tagline: "Respond to message-based events"
quick_start: "https://www.youtube.com/watch?v=Fg1ymPrk6So"
---

# MQ2Events

<!--desc-start-->
MQ2Events allows you to configure a reaction to any text-based event. For example, if you want to `/cry` when the text *"has become ENRAGED"* appears, you're in the right place.

If the event doesn't make text, such as entering a body of water, you should use [MQ2React](../mq2react/index.md).
<!--desc-end-->

## Commands

<a href="cmd-event/">
{% 
  include-markdown "projects/mq2events/cmd-event.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2events/cmd-event.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2events/cmd-event.md') }}

<a href="cmd-eventdebug/">
{% 
  include-markdown "projects/mq2events/cmd-eventdebug.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->"
%}
</a>
:    {% include-markdown "projects/mq2events/cmd-eventdebug.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2events/cmd-eventdebug.md') }}

## Settings

The settings are found in MQ2Events_CharacterName.ini, though they can be added entirely with in-game commands.

You can also use a "generic" MQ2Events.ini

Here's a simple example:

```ini
[mecry]
;the name of the event, can be any short name
- trigger=#1# has become ENRAGED#*#
;the trigger will take anything that appears in #1# and writes it to the TLO ${EventArg1}
- command=/me cowers in fear at the sight of ${EventArg1}
;this is the command that's run, a little emote that includes the mob's name.
```

## Examples

The triggers, **#1#** through **#9#**, act as a wildcard and will take any text that appears in their place and writes it to the corresponding TLO, **${EventArg1}** through **${EventArg9}**. For example, if the text is:
- *For sale: baby shoes, never worn.*
and the trigger is,
- For sale: #1#, never #5#
then following command, `/echo ${EventArg1} ${EventArg5}` will output:
- [MQ2] baby shoes worn.

Here's an example to help characters duck when a monster asks them to,

```ini
[kaelraidduck]
trigger="#*#The ice-encrusted Avatar of War shouts that each of these must bend the knee: #1#, #2#, and #3#.#*#"
command=/if ((${Me.CleanName.Equal[${EventArg1}]}) || (${Me.CleanName.Equal[${EventArg2}]}) || (${Me.CleanName.Equal[${EventArg3}]}) && (!${Me.Ducking})) /multiline ; /gsay Ducking! ; /mqp on ; /attack off ; /twist clear ; /keypress duck
```

Here's one to get notified via EQBC when a character completes a task step, so it notifies you that it completed a step, what Task it was updated in, what step you're currently on, and the instruction for that step.

```ini
[taskupdate]
trigger="Your task '#1#' has been updated#*#"
command=/bc Task Update ${EventArg1} now on step ${Task[${EventArg1}].Step.Index}:${Task[${EventArg1}].Step}.
```

Here's one that uses multiline and variables to congratulate players who have achieved greatness:

```ini
[congorats]
trigger=<SYSTEMWIDE_MESSAGE>: #1# has been defeated by a group of hardy adventurers! Please join us in congratulating #2# along with everyone#*#
command=/if (${congratz} == NULL) /multiline ; /timed 10 /if (!${Defined[congratz]}) /declare congratz timer global 990; /timed 15 /varset congratz 990; /timed 20 /if (!${Defined[randreply]}) /declare randreply int global ${Math.Rand[300]}; /timed 25 /varset randreply ${Math.Rand[300]}; /timed 90 /if (${randreply} >= 0 && ${randreply} <= 100) /tell ${EventArg2} congratz!; /timed 95 /if (${randreply} >= 101 && ${randreply} <= 200) /tell ${EventArg2} nice job!; /timed 100 /if (${randreply} >= 201 && ${randreply} <= 300) /tell ${EventArg2} thanks I hate ${EventArg1}!
```

More examples can be found in this [excellent video](https://www.youtube.com/watch?v=Fg1ymPrk6So), as well as the [support thread](https://www.redguides.com/community/threads/mq2events.66828).

## Top-Level Objects

## [EventArg1](tlo-eventarg1.md)
{% include-markdown "projects/mq2events/tlo-eventarg1.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2events/tlo-eventarg1.md') }}

<h2>Forms</h2>
{% include-markdown "projects/mq2events/tlo-eventarg1.md" start="<!--tlo-forms-start-->" end="<!--tlo-forms-end-->" %}
{% include-markdown "projects/mq2events/tlo-eventarg1.md" start="<!--tlo-linkrefs-start-->" end="<!--tlo-linkrefs-end-->" %}

## [EventArg2](tlo-eventarg2.md)
{% include-markdown "projects/mq2events/tlo-eventarg2.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2events/tlo-eventarg2.md') }}

<h2>Forms</h2>
{% include-markdown "projects/mq2events/tlo-eventarg2.md" start="<!--tlo-forms-start-->" end="<!--tlo-forms-end-->" %}
{% include-markdown "projects/mq2events/tlo-eventarg2.md" start="<!--tlo-linkrefs-start-->" end="<!--tlo-linkrefs-end-->" %}

## [EventArg3](tlo-eventarg3.md)
{% include-markdown "projects/mq2events/tlo-eventarg3.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2events/tlo-eventarg3.md') }}

<h2>Forms</h2>
{% include-markdown "projects/mq2events/tlo-eventarg3.md" start="<!--tlo-forms-start-->" end="<!--tlo-forms-end-->" %}
{% include-markdown "projects/mq2events/tlo-eventarg3.md" start="<!--tlo-linkrefs-start-->" end="<!--tlo-linkrefs-end-->" %}

## [EventArg4](tlo-eventarg4.md)
{% include-markdown "projects/mq2events/tlo-eventarg4.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2events/tlo-eventarg4.md') }}

<h2>Forms</h2>
{% include-markdown "projects/mq2events/tlo-eventarg4.md" start="<!--tlo-forms-start-->" end="<!--tlo-forms-end-->" %}
{% include-markdown "projects/mq2events/tlo-eventarg4.md" start="<!--tlo-linkrefs-start-->" end="<!--tlo-linkrefs-end-->" %}

## [EventArg5](tlo-eventarg5.md)
{% include-markdown "projects/mq2events/tlo-eventarg5.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2events/tlo-eventarg5.md') }}

<h2>Forms</h2>
{% include-markdown "projects/mq2events/tlo-eventarg5.md" start="<!--tlo-forms-start-->" end="<!--tlo-forms-end-->" %}
{% include-markdown "projects/mq2events/tlo-eventarg5.md" start="<!--tlo-linkrefs-start-->" end="<!--tlo-linkrefs-end-->" %}

## [EventArg6](tlo-eventarg6.md)
{% include-markdown "projects/mq2events/tlo-eventarg6.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2events/tlo-eventarg6.md') }}

<h2>Forms</h2>
{% include-markdown "projects/mq2events/tlo-eventarg6.md" start="<!--tlo-forms-start-->" end="<!--tlo-forms-end-->" %}
{% include-markdown "projects/mq2events/tlo-eventarg6.md" start="<!--tlo-linkrefs-start-->" end="<!--tlo-linkrefs-end-->" %}

## [EventArg7](tlo-eventarg7.md)
{% include-markdown "projects/mq2events/tlo-eventarg7.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2events/tlo-eventarg7.md') }}

<h2>Forms</h2>
{% include-markdown "projects/mq2events/tlo-eventarg7.md" start="<!--tlo-forms-start-->" end="<!--tlo-forms-end-->" %}
{% include-markdown "projects/mq2events/tlo-eventarg7.md" start="<!--tlo-linkrefs-start-->" end="<!--tlo-linkrefs-end-->" %}

## [EventArg8](tlo-eventarg8.md)
{% include-markdown "projects/mq2events/tlo-eventarg8.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2events/tlo-eventarg8.md') }}

<h2>Forms</h2>
{% include-markdown "projects/mq2events/tlo-eventarg8.md" start="<!--tlo-forms-start-->" end="<!--tlo-forms-end-->" %}
{% include-markdown "projects/mq2events/tlo-eventarg8.md" start="<!--tlo-linkrefs-start-->" end="<!--tlo-linkrefs-end-->" %}

## [EventArg9](tlo-eventarg9.md)
{% include-markdown "projects/mq2events/tlo-eventarg9.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2events/tlo-eventarg9.md') }}

<h2>Forms</h2>
{% include-markdown "projects/mq2events/tlo-eventarg9.md" start="<!--tlo-forms-start-->" end="<!--tlo-forms-end-->" %}
{% include-markdown "projects/mq2events/tlo-eventarg9.md" start="<!--tlo-linkrefs-start-->" end="<!--tlo-linkrefs-end-->" %}
