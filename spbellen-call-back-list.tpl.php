<?php

/*
 * List of contacts that made a call appointment.
 */

?>

<?php if (empty($data)) : ?>
  <p>Er zijn geen belafspraken</p>
<?php else : ?>
  <table>
  <?php foreach ($data as $campaign_id => $campaign_data) : ?>
    <tr><th colspan="4"><?php print $campaign_data['campaign']['title']; ?></th></tr>
    <?php foreach ($campaign_data['call_back_data'] as $call_back_data) : ?>
      <tr>
        <td><?php print format_date($call_back_data['call_back_time'], 'long'); ?></td>
        <td><?php print $call_back_data['display_name']; ?></td>
        <td><?php print $call_back_data['phone']; ?></td>
        <td><a href="<?php print '/admin/bellen/coach/call_back/' . $call_back_data['target_id']; ?>" target="_blank">nu bellen</a></td>
      </tr>
    <?php endforeach; ?>
  <?php endforeach; ?>
  </table>
<?php endif; ?>
