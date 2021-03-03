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

<?php if(!empty($general_stats['caller'])) : ?>
  <h3>Algemene statistieken</h3>
  <?php $fields = array(
    'answered_completed' => 'Afgerond',
    'answered_call_back' => 'Terugbellen',
    'call_appointment' => 'Belafspraak',
    'skipped' => 'Overgeslagen',
    'wrong_number' => 'Verkeerd nummer',
    'blacklisted' => 'Nooit meer bellen',
    'empty' => '',
    'total' => 'Totaal (exclusief niet opgenomen)',
    'not_answered' => 'Niet opgenomen',
  ); ?>
  <?php $data_sets = array(
    'caller',
    'caller_campaign',
    'campaign',
    'caller_campaign_date',
    'campaign_date',
    'caller_date',
    'date',
  ); ?>
  <table style="width: auto">
    <tr>
      <th></th>
      <th colspan="3">beller&nbsp;totaal</th>
      <?php if (!empty($general_stats['caller_campaign'])) : ?>
        <th colspan="3">beller&nbsp;campagne</th>
      <?php endif; ?>
      <?php if (!empty($general_stats['campaign'])) : ?>
        <th colspan="3">campagne</th>
      <?php endif; ?>
      <?php if (!empty($general_stats['caller_campaign_date'])) : ?>
        <th colspan="3">beller&nbsp;campagne<br/><?php print($general_stats['caller_campaign_date']['call_date']); ?></th>
      <?php endif; ?>
      <?php if (!empty($general_stats['campaign_date'])) : ?>
        <th colspan="3">campagne<br/><?php print($general_stats['campaign_date']['call_date']); ?></th>
      <?php endif; ?>
      <?php if (!empty($general_stats['caller_date'])) : ?>
        <th colspan="3">beller<br/><?php print($general_stats['date']['call_date']); ?></th>
      <?php endif; ?>
      <?php if (!empty($general_stats['date'])) : ?>
        <th colspan="3">alle<br/><?php print($general_stats['date']['call_date']); ?></th>
      <?php endif; ?>
    </tr>
    <?php foreach ($fields as $field_key => $field_title): ?>
      <tr>
        <td width="200"><?php print $field_title; ?></td>
        <?php foreach ($data_sets as $data_set): ?>
          <?php if (!empty($general_stats[$data_set])) : ?>
            <td class="numeric" width="1"><?php print $general_stats[$data_set][$field_key]; ?></td>
            <td class="numeric" width="1"><?php print $general_stats[$data_set][$field_key . '_percentage']; ?><?php $percentage = $field_key == 'empty' ? '&nbsp;' : '%'; print $percentage; ?></td>
            <td></td>
          <?php endif; ?>
        <?php endforeach; ?>
      </tr>
    <?php endforeach; ?>
  </table>
  <?php if (empty($general_stats['campaign'])) : ?>
    <h3>Beltijden</h3>
    <?php $fields = spbellen_action_translate(); ?>
    <?php $data_sets = array(
      'caller',
      'caller_campaign',
      'caller_campaign_date',
      'caller_date',
    ); ?>
    <table style="width: auto">
      <tr>
        <th></th>
        <th colspan="2">beller&nbsp;totaal</th>
        <?php if (!empty($general_stats['caller_campaign'])) : ?>
          <th colspan="2">beller&nbsp;campagne</th>
        <?php endif; ?>
        <?php if (!empty($general_stats['caller_campaign_date'])) : ?>
          <th colspan="2">beller&nbsp;campagne<br/><?php print($general_stats['caller_campaign_date']['call_date']); ?></th>
        <?php endif; ?>
        <?php if (!empty($general_stats['caller_date'])) : ?>
          <th colspan="2">beller<br/><?php print($general_stats['date']['call_date']); ?></th>
        <?php endif; ?>
      </tr>
      <?php foreach ($fields as $field_key => $field_title): ?>
        <tr>
          <td width="200"><?php print $field_title; ?></td>
          <?php foreach ($data_sets as $data_set): ?>
            <?php if (!empty($general_stats[$data_set]['call_times'])) : ?>
              <td class="numeric" width="1"><?php print $general_stats[$data_set]['call_times'][$field_key]; ?></td>
              <td></td>
            <?php endif; ?>
          <?php endforeach; ?>
        </tr>
      <?php endforeach; ?>
    </table>
  <?php endif; ?>
<?php endif; ?>

<?php if (!empty($result_stats['campaign'])) : ?>
  <?php $data_sets = array(
    'caller_campaign',
    'campaign',
    'caller_campaign_date',
    'campaign_date',
  ); ?>
  <?php if (!empty($result_stats['campaign'])) : ?>
    <p><br/></p>
    <h3>Resultaat statistieken</h3>
    <p>(Aantal - Percentage vragen afgerond - percentage telefoon opgenomen)</p>
    <?php foreach ($result_stats['campaign']['components'] as $cid => $data) : ?>
      <p><strong><?php print($data['name']); ?></strong></p>
      <table style="width: auto">
        <tr>
          <th></th>
          <?php if (!empty($result_stats['caller_campaign'])) : ?>
            <th colspan="4">beller campagne</th>
          <?php endif; ?>
          <th colspan="4">campagne</th>
          <?php if (!empty($result_stats['caller_campaign_date'])) : ?>
            <th colspan="4">beller campagne<br/><?php print $result_stats['caller_campaign_date']['call_date']; ?></th>
          <?php endif; ?>
          <?php if (!empty($result_stats['campaign_date'])) : ?>
            <th colspan="4">campagne<br/><?php print $result_stats['campaign_date']['call_date']; ?></th>
          <?php endif; ?>
        </tr>
        <?php foreach ($data['options'] as $key => $option) : ?>
          <tr>
            <td width="200"><?php print($option); ?></td>
            <?php foreach ($data_sets as $data_set) : ?>
              <?php if (!empty($result_stats[$data_set])) : ?>
                <td class="numeric" width="1"><?php print($result_stats[$data_set]['components'][$cid]['stats'][$key]); ?></td>
                <td class="numeric"><?php print($result_stats[$data_set]['components'][$cid]['stats'][$key . '_percentage_answered_completed']); ?>%</td>
                <td class="numeric"><?php print($result_stats[$data_set]['components'][$cid]['stats'][$key . '_percentage_answered']); ?>%</td>
                <td></td>
              <?php endif; ?>
            <?php endforeach; ?>
          </tr>
        <?php endforeach; ?>
      </table>
    <?php endforeach; ?>
  <?php endif; ?>
<?php endif; ?>
