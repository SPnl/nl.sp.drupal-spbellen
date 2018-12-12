<?php
/*
 * Available variables:
 *   campaign_stats
 *     title
 *     total
 *     blacklisted
 *     call_back
 *     completed
 *     wrong_number
 */
?>

<?php if(!empty($general_stats_caller)) : ?>
  <h3><?php print($general_stats_caller_campaign['campaign_title']); ?></h3>
  <h3><?php print($general_stats_caller['caller_display_name']); ?></h3>
  <p><br/></p>
  <h3>Algemene statistieken</h3>
  <table style="width: auto">
    <tr>
      <th></th>
      <th colspan="3">beller&nbsp;totaal</th>
      <th colspan="3">beller&nbsp;campagne</th>
      <th colspan="3">campagne</th>
    </tr>
    <tr>
      <td width="200">Afgerond</td>
      <td class="numeric" width="1"><?php print $general_stats_caller['completed']; ?></td>
      <td class="numeric" width="1"><?php print $general_stats_caller['completed_percentage']; ?>%</td>
      <td></td>
      <td class="numeric" width="1"><?php print $general_stats_caller_campaign['completed']; ?></td>
      <td class="numeric" width="1"><?php print $general_stats_caller_campaign['completed_percentage']; ?>%</td>
      <td></td>
      <td class="numeric" width="1"><?php print $general_stats_campaign['completed']; ?></td>
      <td class="numeric" width="1"><?php print $general_stats_campaign['completed_percentage']; ?>%</td>
      <td></td>
    </tr>
    <tr>
      <td>Terugbellen</td>
      <td class="numeric"><?php print $general_stats_caller['call_back']; ?></td>
      <td class="numeric"><?php print $general_stats_caller['call_back_percentage']; ?>%</td>
      <td></td>
      <td class="numeric"><?php print $general_stats_caller_campaign['call_back']; ?></td>
      <td class="numeric"><?php print $general_stats_caller_campaign['call_back_percentage']; ?>%</td>
      <td></td>
      <td class="numeric"><?php print $general_stats_campaign['call_back']; ?></td>
      <td class="numeric"><?php print $general_stats_campaign['call_back_percentage']; ?>%</td>
      <td></td>
    </tr>
    <tr>
      <td>Verkeerd nummer</td>
      <td class="numeric"><?php print $general_stats_caller['wrong_number']; ?></td>
      <td class="numeric"><?php print $general_stats_caller['wrong_number_percentage']; ?>%</td>
      <td></td>
      <td class="numeric"><?php print $general_stats_caller_campaign['wrong_number']; ?></td>
      <td class="numeric"><?php print $general_stats_caller_campaign['wrong_number_percentage']; ?>%</td>
      <td></td>
      <td class="numeric"><?php print $general_stats_campaign['wrong_number']; ?></td>
      <td class="numeric"><?php print $general_stats_campaign['wrong_number_percentage']; ?>%</td>
      <td></td>
    </tr>
    <tr>
      <td>Nooit meer bellen</td>
      <td class="numeric"><?php print $general_stats_caller['blacklisted']; ?></td>
      <td class="numeric"><?php print $general_stats_caller['blacklisted_percentage']; ?>%</td>
      <td></td>
      <td class="numeric"><?php print $general_stats_caller_campaign['blacklisted']; ?></td>
      <td class="numeric"><?php print $general_stats_caller_campaign['blacklisted_percentage']; ?>%</td>
      <td></td>
      <td class="numeric"><?php print $general_stats_campaign['blacklisted']; ?></td>
      <td class="numeric"><?php print $general_stats_campaign['blacklisted_percentage']; ?>%</td>
      <td></td>
    </tr>
    <tr>
      <td>Totaal</td>
      <td class="numeric"><?php print $general_stats_caller['total']; ?></td>
      <td class="numeric">100%</td>
      <td></td>
      <td class="numeric"><?php print $general_stats_caller_campaign['total']; ?></td>
      <td class="numeric"><?php print $general_stats_caller_campaign['total_percentage']; ?>%</td>
      <td></td>
      <td class="numeric"><?php print $general_stats_campaign['total']; ?></td>
      <td class="numeric"><?php print $general_stats_campaign['total_percentage']; ?>%</td>
      <td></td>
    </tr>
  </table>
<?php endif; ?>

<?php if (!empty($result_stats_caller_campaign)) : ?>
  <p><br/></p>
  <h3>Resultaat statistieken</h3>
  <?php foreach ($result_stats_caller_campaign['components'] as $cid => $data) : ?>
    <p><strong><?php print($data['name']); ?></strong></p>
    <table style="width: auto">
      <tr>
        <th></th>
        <th colspan="2">beller</th>
        <th colspan="2">campagne</th>
      </tr>
      <?php foreach ($data['options'] as $key => $option) : ?>
        <tr>
          <td width="200"><?php print($option); ?></td>
          <td class="numeric" width="1"><?php print($data['stats'][$key]); ?></td>
          <td class="numeric"><?php print($data['stats'][$key. '_percentage']); ?>%</td>
          <td class="numeric" width="1"><?php print($result_stats_campaign['components'][$cid]['stats'][$key]); ?></td>
          <td class="numeric"><?php print($result_stats_campaign['components'][$cid]['stats'][$key . '_percentage']); ?>%</td>
        </tr>
      <?php endforeach; ?>
    </table>
  <?php endforeach; ?>
<?php endif; ?>
